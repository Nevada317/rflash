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

/*
static void debug() {
	uint32_t count = 555;
	datasegment_t** arr = DATASEG_Enumerate(root, &count);

	printf("List countains %d records:\n", count);
	for (uint32_t i = 0; i < count; i++) {
		// printf(" Record %d: %08x\n", i, (uint32_t) arr[i]);
		printf(" Offset %08x, Length %d\n", arr[i]->Offset, arr[i]->Length);

		uint16_t left = arr[i]->Length;
		uint8_t slen = 0;
		uint32_t ii = 0;
		while (left) {
			if (!slen) {
				printf("%08lx  ", arr[i]->Offset + ii);
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
	} else if (!strcmp(arg_memory, "hfuse")) {
		proto.memory_type = MEM_HFUSE;
	} else if (!strcmp(arg_memory, "efuse")) {
		proto.memory_type = MEM_EFUSE;
	} else if (!strcmp(arg_memory, "lock")) {
		proto.memory_type = MEM_LOCK;
	} else if (!strcmp(arg_memory, "flash")) {
		proto.memory_type = MEM_FLASH;
	} else if (!strcmp(arg_memory, "eeprom")) {
		proto.memory_type = MEM_EEPROM;
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

	proto.arg_string = strdup(arg_value);

	mem_task_t* newrecord = QUEUE_NewRecord(&tasks_root);
	memcpy(newrecord, &proto, sizeof(proto));

	(void) arg_value;
	(void) arg_type;
}

static void arg_1(char key, char* arg) {
	(void)key;
	(void)arg;
	if (!key) key = '?';
	printf("KEY %c = %s\n", key, arg);
}

arg_key_t Keys[] = {
	{.needs_arg = true, .handler = arg_1},
	// {.key = 'c', .needs_arg = true,  .handler = arg_1},
	{.key = 'p', .needs_arg = true,  .handler = arg_1},
	{.key = ' ', .needs_arg = true,  .handler = arg_1},
	{.key = 'U', .needs_arg = true,  .handler = arg_U},
	{.key = 'e', .needs_arg = false, .handler = arg_1},
	{0}
};

int main(int argc, char *argv[]) {
	(void)argc;

	ARGS_ParseArgsByList(argv, &AppName, Keys);
	printf("App name: %s\n", AppName);

	// IHEX_AppendHex(&root, "test.hex");
	// DATASEG_Fuse(root);
	// debug();

	QUEUE_Destroy(&tasks_root);
	return 0;
}

