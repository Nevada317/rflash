#include "dataseg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


datasegment_t* root;

int main() {
	datasegment_t* ptr;
	ptr = DATASEG_AutoAppend(&root);
	DATASEG_Alloc(ptr, 10, 10);
	ptr = DATASEG_AutoAppend(&root);
	DATASEG_Alloc(ptr, 20, 10);
	ptr = DATASEG_AutoAppend(&root);
	DATASEG_Alloc(ptr, 30, 10);
	ptr = DATASEG_AutoAppend(&root);
	DATASEG_Alloc(ptr, 40, 10);
	ptr = DATASEG_AutoAppend(&root);
	DATASEG_Alloc(ptr, 55, 10);
	ptr = DATASEG_AutoAppend(&root);
	DATASEG_Alloc(ptr, 65, 10);

	DATASEG_Merge(root);
	// DATASEG_Cleanup(root);

	datasegment_t** arrX = DATASEG_Enumerate(root, 0);
	// DATASEG_Remove(arrX[0]);
	free(arrX);


	uint32_t count = 555;
	datasegment_t** arr = DATASEG_Enumerate(root, &count);

	printf("List countains %d records:\n", count);
	for (uint32_t i = 0; i < count; i++) {
		// printf(" Record %d: %08x\n", i, (uint32_t) arr[i]);
		printf(" Offset %08x, Length %d, Ptr %p\n", arr[i]->Offset, arr[i]->Length, arr[i]->Payload);
	}
	// printf(" Record %d: %08x\n", count, (uint32_t) arr[count]);
	free(arr);

	return 0;
}
