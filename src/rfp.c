#include "rfp.h"
#include <stdlib.h>
#include <string.h>


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

static uint16_t CRC_Dummy(void* buffer, uint8_t len) {
	(void) buffer;
	(void) len;
	return 0x1234;
}

rfp_flexbuffer_t * RFP_CreateParcel(rfp_command_t command, uint8_t index, rfp_buffer_t* buffer) {
	uint16_t UnstuffedLength = 1+1+1+128+2; // See "Transport parcel"
	uint8_t idx = index;
	uint8_t len = 0;
	void* src = 0;

	switch (command) {
		case RFP_CMD_POLL:
		case RFP_CMD_RESET:
			idx = 0xFF;
			break;
		case RFP_CMD_Data0: // Used by imitator
		case RFP_CMD_Add0:
			if (buffer) {
				const uint16_t psize = buffer->PayloadSize ? buffer->PayloadSize : 256;
				src = &buffer->Payload[0];
				len = psize > 128 ? 128 : psize;
			}
			break;
		case RFP_CMD_Data1: // Used by imitator
		case RFP_CMD_Add1:
			if (buffer) {
				const uint16_t psize = buffer->PayloadSize ? buffer->PayloadSize : 256;
				src = &buffer->Payload[128];
				len = psize > 128 ? psize - 128 : 0;
			}
			break;
		case RFP_CMD_Add2:
			if (buffer) {
				src = buffer->Payload + sizeof(buffer->Payload);
				len = sizeof(*buffer) - sizeof(buffer->Payload);
			}
			break;
		case RFP_CMD_REPORT:
			src = buffer->Payload;
			len = buffer->PayloadSize;
			idx = 0xFF;
			break;
		// Those has no args
		case RFP_CMD_Get0:
		case RFP_CMD_Get1:
		case RFP_CMD_Ack:
			break;
	}
	if (!src) len = 0;

	uint8_t InitialBuffer[UnstuffedLength];
	uint16_t w = 0;
	InitialBuffer[w++] = (uint8_t) command;
	InitialBuffer[w++] = (uint8_t) idx;
	InitialBuffer[w++] = (uint8_t) len;
	if (src && len) {
		memcpy(&InitialBuffer[w], src, len);
		w += len;
	}
	uint16_t CRC = CRC_Dummy(InitialBuffer, w);
	InitialBuffer[w++] = (CRC & 0xFF);
	InitialBuffer[w++] = (CRC >> 8);
	UnstuffedLength = w;

	uint16_t DoubleLength = 1 + 2*UnstuffedLength;
	rfp_flexbuffer_t * res = calloc(1, sizeof(rfp_flexbuffer_t) + DoubleLength);
	w = 0;

	res->Data[w++] = RFP_STX;
	for (uint16_t r = 0; r < UnstuffedLength; r++) {
		if (InitialBuffer[r] == RFP_STX) {
			res->Data[w++] = RFP_ESC;
			res->Data[w++] = RFP_ESTX;
		} else if (InitialBuffer[r] == RFP_ESC) {
			res->Data[w++] = RFP_ESC;
			res->Data[w++] = RFP_EESC;
		} else {
			res->Data[w++] = InitialBuffer[r];
		}
	}
	res->Length = w;
	rfp_flexbuffer_t * newres = realloc(res, sizeof(rfp_flexbuffer_t) + res->Length);
	if (newres) {
		return newres;
	} else {
		free(res);
		return 0;
	}
}
