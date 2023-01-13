#include "dataseg.h"
#include "ihex.h"
#include "args.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

char* AppName = NULL;

datasegment_t* root;

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
	{.key = 'U', .needs_arg = true,  .handler = arg_1},
	{.key = 'e', .needs_arg = false, .handler = arg_1},
	{0}
};

static bool DisplayDebug1(char chr, char * string) {
	if (!chr) {
		printf("End of arguments list\n");
		return false;
	}
	if (chr != ' ' && !string && chr != 'e') {
		return false;
	}

	printf("Argument: %c=%s\n", chr, string);
	return true;
}

int main(int argc, char *argv[]) {
	(void)argc;

	ARGS_ParseArgs(argv, &AppName, &DisplayDebug1);
	printf("App name: %s\n", AppName);
	ARGS_ParseArgsByList(argv, &AppName, Keys);
	printf("App name: %s\n", AppName);

	// IHEX_AppendHex(&root, "test.hex");
	// DATASEG_Fuse(root);
	// debug();

	return 0;
}

