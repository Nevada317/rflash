#include "ihex.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MaxStringLength 4096

typedef struct {
	uint32_t address_offset;
	bool non_empty;
	bool terminated;
} ihex_context_t;

typedef enum {
	IHEX_Type_Data        = 0x00,
	IHEX_Type_EOF         = 0x01,
	IHEX_Type_ExtSegAddr  = 0x02,
	IHEX_Type_StaSegAddr  = 0x03,
	IHEX_Type_ExtLinAddr  = 0x04,
	IHEX_Type_StaLinAddr   = 0x05
} ihex_rtype_t;

static uint16_t _read_hexBE(const char* str, uint8_t bytes) {
	uint16_t temp = 0;
	for (uint8_t i = 0; i < (2*bytes); i++) {
		char ch = *(str+i);
		temp <<= 4;
		// Transition lowercase to uppercase
		if (ch >= 0x60) ch -= 0x20;

		if (ch < '0') {
			// Skip step - below 0
		} else if (ch <= '9') {
			// Regular digit
			temp |= ch - '0';
		} else if (ch < 'A') {
			// Skip step - not a letter
		} else if (ch <= 'F') {
			// HEX letter
			temp |= ch - 'A' + 10;
		}
	}
	return temp;
}

static bool _try_to_add_seg(datasegment_t** ptr_root, char* line, ihex_context_t* ctx) {
	if (!ptr_root) return false;
	if (!line) return false;

	char* rptr = line;
	if (*(rptr++) != ':') {
		printf("SKIP: Ignored HEX line:\n%s\n", line);
		return false;
	}
	if (strlen(rptr) < (2*(1+2+1+1))) {
		printf("SKIP: Too short line:\n%s\n", line);
		return false;
	}

#ifdef DEBUG_IHEX
	printf("\nParsing line:\n%s\n", line);
#endif

	uint8_t payload_len = _read_hexBE(rptr, 1);
	rptr += 2* 1;
#ifdef DEBUG_IHEX
	printf("Length %02x, ", payload_len);
#endif

	if ((int)strlen(line) != (2*(1+2+1+payload_len+1) + 1)) {
		printf("SKIP: Bad line length:\n%s\n", line);
		return false;
	}

	char* chptr = line+1;
	uint8_t check = 0;
	for (uint16_t i = 0; i < (1+2+1+payload_len+1); i++) {
		check += _read_hexBE(chptr, 1);
		chptr += 2;
	}
	if (check) {
		printf("SKIP: Checksum does not match:\n%s\n", line);
		return false;
	}

	uint32_t addr = _read_hexBE(rptr, 2);
	rptr += 2* 2;
#ifdef DEBUG_IHEX
	printf("Addr %04x, ", addr);
#endif

	ihex_rtype_t type = (ihex_rtype_t) _read_hexBE(rptr, 1);
	rptr += 2* 1;
	switch (type) {
		case IHEX_Type_Data:
			// Only possible break
			break;
		case IHEX_Type_EOF:
			if (payload_len != 0) {
				printf("SKIP: Non-empty EOF:\n%s\n", line);
				return false;
			}
			ctx->terminated = true;
			return true;
		case IHEX_Type_ExtSegAddr:
			ctx->address_offset = (uint32_t) 16 * _read_hexBE(rptr, 2);
			return true;
		case IHEX_Type_StaSegAddr:
			printf("SKIP: Start address instruction not supported:\n%s\n", line);
			return false;
		case IHEX_Type_ExtLinAddr:
			ctx->address_offset = _read_hexBE(rptr, 2) << 16;
			return true;
		case IHEX_Type_StaLinAddr:
			printf("SKIP: Start address instruction not supported:\n%s\n", line);
			return false;
		default:
			printf("Unknown record type %02x:\n%s\n", (uint8_t)type, line);
			return false;
	}
	// Here we are if data line found
	if (payload_len == 0) {
		printf("SKIP: Empty data field:\n%s\n", line);
		return false;
	}

	datasegment_t* seg = DATASEG_AutoAppend(ptr_root);
	if (!seg) return false;
	seg = DATASEG_Alloc(seg, ctx->address_offset + addr, payload_len);
	if (!seg->Payload) return false;

	uint8_t* wptr = seg->Payload;
	for (uint16_t i = 0; i < payload_len; i++) {
		*(wptr++) = _read_hexBE(rptr, 1);
		rptr += 2* 1;
	}

	return true;
}

bool IHEX_AppendHex(datasegment_t** ptr_root, char* filename) {
	FILE *fp;
	fp = fopen(filename , "r");
	if (!fp) {
		printf("Error opening file '%s'", filename);
		return false;
	}
	ihex_context_t ctx = {0};
	char str[MaxStringLength+1];
	char* lfptr;
	while (1) {
		// Clear string
		str[0] = '\0';
		// Try to read line
		if (!fgets(str, MaxStringLength+1, fp)) break;
		// Remove trailing newline
		if ((lfptr = strchr(str, '\n'))) *lfptr = '\0';
		if ((lfptr = strchr(str, '\r'))) *lfptr = '\0';
		// Remove GNU-style comments
		if ((lfptr = strchr(str, '#'))) *lfptr = '\0';
		// Remove trailing space
		if ((lfptr = strchr(str, ' '))) *lfptr = '\0';
		// Skip empty lines
		if (!strlen(str)) continue;
		// Parse line
		_try_to_add_seg(ptr_root, str, &ctx);
	}
	return ctx.non_empty;
}
