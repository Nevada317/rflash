#include "rfp.h"
#include <stdlib.h>


rfp_list_t* RFP_LIST_NewRecord(rfp_list_t** root_ptr) {
	if (!root_ptr) return 0;

	rfp_list_t* newrecord = calloc(1, sizeof(rfp_list_t));

	rfp_list_t* cur = *root_ptr;
	if (cur) {
		if (cur->root) (*root_ptr)->root = cur->root;
		while (cur->next) {
			cur = cur->next;
		}
		cur->next = newrecord;
		newrecord->index = cur->index + 1;
		newrecord->root = *root_ptr;
	} else {
		*root_ptr = newrecord;
	}
	(*root_ptr)->count = newrecord->index + 1;
	return newrecord;
}

void RFP_AppendCRC(rfp_buffer_t* Buffer) {
	(void)Buffer;
}

rfp_flexbuffer_t * RFP_CreateParcel(rfp_command_t command, uint8_t index, rfp_buffer_t buffer) {
	// TODO: Create parcel with all needed data
	// TODO: Calculate CRC
	// TODO: Allocate rfp_flexbuffer_t with enough room
	// TODO: Perform stuffing
	// TODO: Shorten rfp_flexbuffer_t by realloc
	return 0;
}
