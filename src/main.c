#include "dataseg.h"
#include "ihex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

datasegment_t* root;


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



int main() {
	datasegment_t* ptr;

	IHEX_AppendHex(&root, "test.hex");
	DATASEG_Fuse(root);
	debug();

	ptr = DATASEG_AutoAppend(&root);
	DATASEG_Alloc(ptr, 10, 10);
	memset(ptr->Payload, 0x11, ptr->Length);
	// ptr = DATASEG_AutoAppend(&root);
	// DATASEG_Alloc(ptr, 20, 50);

	DATASEG_Fuse(root);
	debug();

	return 0;
}

