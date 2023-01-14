#include "dataseg.h"
#include "ihex.h"
#include "args.h"
#include "queue_logic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

char* AppName = NULL;

mem_task_t* tasks_root = 0;

datasegment_t* global_root_FLASH = 0;
datasegment_t* global_root_EEPROM = 0;


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


static void arg_U(char key, char* arg) {
	if ((key != 'U') || !arg) return;
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
		IHEX_AppendHex(newrecord->root_ptr, newrecord->arg_string);
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
static void arg_DUMMY(char key, char* arg) {
	if (!key) key = '?';
	printf("Ignoring argument %c = %s\n", key, arg);
}

arg_key_t Keys[] = {
	{.needs_arg = true, .handler = arg_1},
	// {.key = 'c', .needs_arg = true,  .handler = arg_1},
	{.key = 'c', .needs_arg = true,  .handler = arg_DUMMY}, // Ignore
	{.key = 'p', .needs_arg = true,  .handler = arg_DUMMY}, // Ignore
	{.key = 'B', .needs_arg = true,  .handler = arg_DUMMY}, // Ignore
	{.key = ' ', .needs_arg = true,  .handler = arg_1},
	{.key = 'U', .needs_arg = true,  .handler = arg_U},
	{.key = 'e', .needs_arg = false, .handler = arg_1},
	{0}
};

int main(int argc, char *argv[]) {
	(void)argc;

	ARGS_ParseArgsByList(argv, &AppName, Keys);
	printf("App name: %s\n", AppName);
	check_queue(tasks_root);

	// IHEX_AppendHex(&root, "test.hex");
	// debug();

	QUEUE_Destroy(&tasks_root);
	return 0;
}

