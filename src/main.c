#include "dataseg.h"
#include "ihex.h"
#include "args.h"
#include "rfp.h"
#include "rfp_queue.h"
#include "queue_logic.h"
#include "avr/devices.h"
#include "tcp_client.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

char* AppName = NULL;

mem_task_t* tasks_root = 0;
rfp_list_t* rfp_queue = 0;

datasegment_t* global_root_FLASH = 0;
datasegment_t* global_root_EEPROM = 0;

const avr_device_t* AVR_Device = 0;

static bool Failed = false;

static bool SkipSignatureCheck = false;
static bool SkipErase = false;

static struct Connection_Params {
	uint32_t Baud;
	char* PortString;
} Connection = {
	.Baud = 115200,
};

/*
static void debug() {
	uint32_t count = 555;
	datasegment_t** arr = DATASEG_Enumerate(global_root_FLASH, &count);

	printf("List countains %d records:\n", count);
	for (uint32_t i = 0; i < count; i++) {
		// printf(" Record %d: %08x\n", i, (uint32_t) arr[i]);
		printf(" Offset %08x, Length %d\n", arr[i]->Offset, arr[i]->Length);

		uint16_t left = arr[i]->Length;
		uint8_t slen = 0;
		uint32_t ii = 0;
		while (left) {
			if (!slen) {
				printf("%08x  ", arr[i]->Offset + ii);
			}
			printf(" %02X", *((uint8_t*)arr[i]->Payload+(ii++)));
			if (slen++ == 15) {
				slen = 0;
				printf("\n");
			}
			left--;
		}
		printf("\n");
	}
	// printf(" Record %d: %08x\n", count, (uint32_t) arr[count]);
	free(arr);
}
*/

void my_cb(void* data, int length) {
	printf("Rx cb: %d @ %p\n", length, data);
	// server_send("test\n", 5);
}


static void arg_e(char key, char* arg) {
	if (key != 'e') return;
	if (Failed) return;
	(void)arg;
	mem_task_t* newrecord = QUEUE_NewRecord(&tasks_root);
	newrecord->memory_operation = MEM_OPER_ERASE;
	SkipErase = true;
}

static void arg_D(char key, char* arg) {
	if (key != 'D') return;
	if (Failed) return;
	(void)arg;
	SkipErase = true;
}

static void arg_F(char key, char* arg) {
	if (key != 'F') return;
	if (Failed) return;
	(void)arg;
	SkipSignatureCheck = true;
}

static void arg_P(char key, char* arg) {
	if (key != 'P') return;
	if (Failed) return;
	Connection.PortString = strdup(arg);
}

static void arg_p(char key, char* arg) {
	if (key != 'p') return;
	if (Failed) return;
	AVR_Device = AVR_DEVICE_Search(arg);

	if (!AVR_Device) {
		printf("Error parsing argument: -%c %s\nUnknown chip\n", key, arg);
		Failed = true;
		return;
	}
	if (AVR_Device->name) {
		printf("Selected chip: %s\n", AVR_Device->name);
	}
}

static void arg_U(char key, char* arg) {
	if ((key != 'U') || !arg) return;
	if (Failed) return;
	// printf("KEY %c = %s\n", key, arg);

	mem_task_t proto = {0};
	char* string_copy = strdup(arg);
	char* arg_memory = strtok(string_copy, ":"); // Memory name
	char* arg_oper   = strtok(NULL, ":"); // Operation
	char* arg_value  = strtok(NULL, ":"); // Value
	char* arg_type   = strtok(NULL, ":"); // Type

	if (!strcmp(arg_memory, "lfuse")) {
		proto.memory_type = MEM_LFUSE;
		proto.isFuse = true;
	} else if (!strcmp(arg_memory, "hfuse")) {
		proto.memory_type = MEM_HFUSE;
		proto.isFuse = true;
	} else if (!strcmp(arg_memory, "efuse")) {
		proto.memory_type = MEM_EFUSE;
		proto.isFuse = true;
	} else if (!strcmp(arg_memory, "lock")) {
		proto.memory_type = MEM_LOCK;
		proto.isFuse = true;
	} else if (!strcmp(arg_memory, "flash")) {
		proto.memory_type = MEM_FLASH;
		proto.isArray = true;
	} else if (!strcmp(arg_memory, "eeprom")) {
		proto.memory_type = MEM_EEPROM;
		proto.isArray = true;
	} else {
		printf("Error parsing argument: -%c %s\nUnknown memory type: %s\n", key, arg, arg_memory);
		return;
	}

	if (!strcmp(arg_oper, "w")) {
		proto.memory_operation = MEM_OPER_WRITE;
	} else if (!strcmp(arg_oper, "v")) {
		proto.memory_operation = MEM_OPER_VERIFY;
	} else if (!strcmp(arg_oper, "r")) {
		proto.memory_operation = MEM_OPER_READ;
	} else {
		printf("Error parsing argument: -%c %s\nUnknown action: %s\n", key, arg, arg_oper);
		return;
	}

	mem_task_t* newrecord = QUEUE_NewRecord(&tasks_root);
	memcpy(newrecord, &proto, sizeof(proto));

	// No way back. Task added to queue

	newrecord->arg_string = strdup(arg_value);

	if (newrecord->isFuse && (newrecord->memory_operation == MEM_OPER_WRITE)) {
		newrecord->arg_byte = strtol(newrecord->arg_string, NULL, 0);
	}

	if (newrecord->isArray) {
		// Array handlers for both write and read/verify
		// By default - we use own root for task storage
		newrecord->root_ptr = &(newrecord->root_own);
		if (newrecord->memory_operation == MEM_OPER_WRITE) {
			if (newrecord->memory_type == MEM_FLASH) {
				newrecord->root_ptr = &global_root_FLASH;
				if (global_root_FLASH) {
					newrecord->continuation = true;
				}
			}
			if (newrecord->memory_type == MEM_EEPROM) {
				newrecord->root_ptr = &global_root_EEPROM;
				if (global_root_EEPROM) {
					newrecord->continuation = true;
				}
			}
		}
		if (!IHEX_AppendHex(newrecord->root_ptr, newrecord->arg_string)) Failed = true;
	}


	(void) arg_value;
	(void) arg_type;
}

static void arg_1(char key, char* arg) {
	(void)key;
	(void)arg;
	if (!key) key = '?';
	printf("KEY %c = %s\n", key, arg);
}

static void check_queue(mem_task_t* queue) {
	mem_task_t* task = queue;
	mem_task_t* prev = 0;
	while (task) {
		if (Failed) break;
		if (task->continuation) {
			task->root_ptr = 0;
			if (prev) {
				prev->next = task->next;
				free(task);
				task = prev->next;
				continue;
			}
		}
		if (task->root_ptr) {
			DATASEG_Fuse(*(task->root_ptr));
		}
		prev = task;
		task = task -> next;
	}
}

static bool transfer_segment_to_page(rfp_buffer_t* page, datasegment_t* segment) {
	if (!page) return false;
	if (!segment) return false;
	uint16_t PageSize = page->PayloadSize;
	if (!PageSize) PageSize = 256;

	if (segment->Offset >= (page->Address + PageSize)) return false;
	if ((segment->Offset + segment->Length) <= page->Address) return false;

	uint32_t CopyRange_A = segment->Offset;
	uint32_t CopyRange_B = segment->Offset + segment->Length;

	if (CopyRange_A < page->Address) {
		CopyRange_A = page->Address;
	}
	if (CopyRange_B > (page->Address + PageSize)) {
		CopyRange_B = (page->Address + PageSize);
	}
	uint32_t Count = CopyRange_B - CopyRange_A;
	if (!Count) return false;
	memcpy(page->Payload+CopyRange_A-page->Address, segment->Payload+CopyRange_A-segment->Offset, Count);
	return true;
}

void DemoPrintPage(rfp_buffer_t* buffer) {
	uint16_t left = buffer->PayloadSize;
	if (!left) left = 256;
	printf(" Offset %08x, Length %d, Operation %02x, Protocol %02x\n", buffer->Address, left, buffer->Operation, buffer->Protocol);
	uint8_t slen = 0;
	uint32_t ii = 0;
	while (left) {
		if (!slen) {
			printf("%08x  ", buffer->Address + ii);
		}
		printf(" %02X", buffer->Payload[ii++]);
		if (slen++ == 15) {
			slen = 0;
			printf("\n");
		}
		left--;
	}
	if (slen) {
		printf("\n");
	}


}

static void add_task_to_rfp_queue(mem_task_t* task, rfp_list_t** rfp_qptr) {
	rfp_list_t* rfp_list_item = 0;
	rfp_buffer_t* rfp_buf = 0;
	rfp_operation_t oper = 0;

	if (task->memory_operation == MEM_OPER_ERASE) {
		oper = RFP_OPER_Erase;
	} else if (task->memory_operation == MEM_OPER_WRITE) {
		oper = RFP_OPER_Write;
	} else if (task->memory_operation == MEM_OPER_VERIFY) {
		oper = RFP_OPER_Verify;
	} else if (task->memory_operation == MEM_OPER_READ) {
		oper = RFP_OPER_Read;
		// TODO: do something for read!
		printf("Operation: read not supported!\n");
		Failed = true;
		return;
	}

	if (task->isFuse) {
		// printf("Operation: fuse!\n");
		rfp_list_item = RFP_LIST_NewRecord(rfp_qptr);
		rfp_buf = &rfp_list_item->Buffer;
		rfp_buf->Operation = oper;
		rfp_buf->PayloadSize = 1;
		rfp_buf->Payload[0] = task->arg_byte;
		if (task->memory_type == MEM_LFUSE) {
			rfp_buf->Address = OFFSET_FUSE + AVR_Device->off.FuseL;
		} else if (task->memory_type == MEM_HFUSE) {
			rfp_buf->Address = OFFSET_FUSE + AVR_Device->off.FuseH;
		} else if (task->memory_type == MEM_EFUSE) {
			rfp_buf->Address = OFFSET_FUSE + AVR_Device->off.FuseE;
		} else if (task->memory_type == MEM_LOCK) {
			rfp_buf->Address = OFFSET_FUSE + AVR_Device->off.Lock;
		} else {
			printf("Unknown fuse!\n");
			Failed = true;
			return;
		}
	} else if (task->isArray) {
		// printf("Operation: array!\n");
		uint32_t PageSize;
		uint32_t MemoryMaxAddr;
		uint32_t MemoryOffset;
		if (task->memory_type == MEM_FLASH) {
			PageSize = AVR_Device->specs.FLASH_PageSize;
			MemoryMaxAddr = AVR_Device->specs.FLASH_Size;
			MemoryOffset = OFFSET_FLASH + AVR_Device->off.Flash;
		} else if (task->memory_type == MEM_EEPROM) {
			PageSize = AVR_Device->specs.EEPROM_PageSize;
			MemoryMaxAddr = AVR_Device->specs.EEPROM_Size;
			MemoryOffset = OFFSET_EEP + AVR_Device->off.EEPROM;
		} else {
			printf("Unknown memory type!\n");
			Failed = true;
			return;
		}
		if (!MemoryMaxAddr) {
			printf("Trying to write to non-existent memory!\n");
			Failed = true;
			return;
		}
		// If memory exists and is not page-orinented, assume it is 1-byte pages
		if (!PageSize) {PageSize = 1;}
		if (PageSize > 256) {
			printf("Too large page size (0x%x)!\n", PageSize);
			Failed = true;
			return;
		}
		const uint32_t PageMask = ~(PageSize-1);
		datasegment_t* segment_cur = *(task->root_ptr);
		if (!segment_cur) {
			printf("Segment data not found!\n");
			Failed = true;
			return;
		}

		uint32_t CurrentOffset = 0;
		bool MatchCur;
		bool MatchNext;
		bool createnewstart = true;

		datasegment_t* segment_runner = 0;

		while (segment_cur) {
			if (createnewstart) {
				if ((segment_cur->Offset & PageMask) < CurrentOffset) {
					// Segment already covered
					segment_cur = segment_cur->next;
					continue;
				}
				CurrentOffset = segment_cur->Offset & PageMask;
				createnewstart = false;
			}

			rfp_list_item = RFP_LIST_NewRecord(rfp_qptr);
			rfp_buf = &rfp_list_item->Buffer;
			rfp_buf->Operation = oper;
			rfp_buf->PayloadSize = PageSize & 0xFF;
			rfp_buf->Address = CurrentOffset;

			memset(rfp_buf->Payload, 0xFF, sizeof(rfp_buf->Payload));
			MatchCur = transfer_segment_to_page(rfp_buf, segment_cur);
			segment_runner = segment_cur;
			MatchNext = false;
			while ((segment_runner = segment_runner->next)) {
				if (!transfer_segment_to_page(rfp_buf, segment_runner)) break;
				MatchNext = true;
			}

			rfp_buf->Address += MemoryOffset;

			CurrentOffset += PageSize;

			if (CurrentOffset >= (segment_cur->Offset + segment_cur->Length)) {
				segment_cur = segment_cur->next;
				MatchCur = false;
			}
			if (!MatchCur && !MatchNext) {
				createnewstart = true;
			}
		}

	} else if (task->memory_operation == MEM_OPER_ERASE) {
		// printf("Operation: chip erase!\n");
		rfp_list_item = RFP_LIST_NewRecord(rfp_qptr);
		rfp_buf = &rfp_list_item->Buffer;
		rfp_buf->Operation = RFP_OPER_Erase;
	} else {
		printf("Unknown operation in task queue!\n");
		Failed = true;
	}
}

static void fill_rfp_queue(mem_task_t* tasks_queue, rfp_list_t** rfp_qptr) {
	if (Failed) return;
	if (!AVR_Device) return;
	rfp_list_t* rfp_list_item = 0;
	rfp_buffer_t* rfp_buf = 0;

	if (!SkipSignatureCheck && AVR_Device->signature.Length) {
		rfp_list_item = RFP_LIST_NewRecord(rfp_qptr);
		if (!rfp_list_item) {
			Failed = true;
			return;
		}
		rfp_buf = &rfp_list_item->Buffer;
		rfp_buf->PayloadSize = AVR_Device->signature.Length;
		memcpy(rfp_buf->Payload, AVR_Device->signature.Value, rfp_buf->PayloadSize);
		rfp_buf->Operation = RFP_OPER_SigCheck;
	}

	mem_task_t* task = tasks_queue;
	while (task) {
		if (Failed) break;
		add_task_to_rfp_queue(task, rfp_qptr);
		task = task -> next;
	}

	// Release bus
	rfp_list_item = RFP_LIST_NewRecord(rfp_qptr);
	rfp_buf = &rfp_list_item->Buffer;
	rfp_buf->Operation = RFP_OPER_Release;
}

static void sign_rfp_queue(rfp_list_t** rfp_qptr) {
	if (!rfp_qptr) return;
	if (!*rfp_qptr) return;
	rfp_list_t* rfp_item = *rfp_qptr;
	while (rfp_item) {
		rfp_item->Buffer.Protocol = RFP_PROTOCOL_AVR;
		if (AVR_Device->specs.has_extended_address) {
			rfp_item->Buffer.Flags |= Flag_AVR_ExtendedAddress;
		}
		RFP_AppendCRC(&rfp_item->Buffer);
		DemoPrintPage(&rfp_item->Buffer);
		rfp_item = rfp_item->next;
	}
}

static void send_rfp_queue(rfp_list_t** rfp_qptr) {
	// This is stub only. That is not supposed to work with rel RFP

	if (!rfp_qptr) return;
	if (!*rfp_qptr) return;
	rfp_list_t* rfp_item = *rfp_qptr;
	while (rfp_item) {
		server_send(&rfp_item->Buffer, sizeof(rfp_item->Buffer));
		rfp_item = rfp_item->next;
	}
}

static void arg_DUMMY(char key, char* arg) {
	if (Failed) return;
	if (!key) key = '?';
	printf("Ignoring argument %c = %s\n", key, arg);
}

arg_key_t Keys[] = {
	{.needs_arg = true, .handler = arg_1}, // Default handler
	{.key = 'c', .needs_arg = true,  .handler = arg_DUMMY}, // Ignore
	{.key = 'B', .needs_arg = true,  .handler = arg_DUMMY}, // Ignore
	{.key = 'p', .needs_arg = true,  .handler = arg_p}, // Part
	{.key = 'P', .needs_arg = true,  .handler = arg_P}, // Port
	{.key = 'U', .needs_arg = true,  .handler = arg_U}, // Command
	{.key = 'e', .needs_arg = false, .handler = arg_e}, // Chip erase
	{.key = 'D', .needs_arg = false, .handler = arg_D}, // Disable chip erase
	{.key = 'F', .needs_arg = false, .handler = arg_F}, // Disable signature check
	{0}
};

int main(int argc, char *argv[]) {
	(void)argc;

	Failed = false;
	do { // Breakable block
		ARGS_ParseArgsByList(argv, &AppName, Keys);

		if (Failed) break;
		if (!AVR_Device) {
			printf("ERROR: No target specified. Use key -p\n");
			Failed = true;
		}
		if (Failed) break;
		check_queue(tasks_root);

		if (Failed) break;
		fill_rfp_queue(tasks_root, &rfp_queue);

		if (Failed) break;
		sign_rfp_queue(&rfp_queue);

		if (Failed) break;
		Failed |= !connect_tcp_qualified("127.0.0.1", 8080, my_cb);

		// if (Failed) break;
		// send_rfp_queue(&rfp_queue);

		RFP_Queue_Init();
		RFP_Queue_StartTask(rfp_queue);
		RFP_Queue_Wait();

		// rfp_flexbuffer_t * temp = RFP_CreateParcel(RFP_CMD_Add0, 0x55, &rfp_queue->Buffer);
		// if (temp) {
		// 	server_send(&temp->Data, temp->Length);
		// }
	} while (0);

	if (Failed) {
		printf("\n\nRefusing to continue because of errors\n\n");
	}

	QUEUE_Destroy(&tasks_root);
	DATASEG_Destroy(&global_root_EEPROM);
	DATASEG_Destroy(&global_root_FLASH);

	disconnect_tcp();

	return Failed ? 1 : 0;
}

