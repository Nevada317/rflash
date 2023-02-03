#include "rfp.h"
#include "rfp_transport_decoder.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define BUFFERSIZE_MAX 64000

#define RxIndex_Stopped   -1
#define RxIndex_Overflow  -2
#define RxIndex_BadCRC    -3
#define RxIndex_Complete  -4

static struct {
	void (*decoder_cb)(void* data, int length);
	bool (*decoder_crc_checker)(void* data, uint8_t length);
	bool Escape;
	union {
		uint8_t Buffer[1+1+1+1+128+2];
		rfp_transport_rx_t Parcel;
	};
	int RxIndex;
} RFP_Transport_Decoder_State;

#define U RFP_Transport_Decoder_State

void RFP_Transport_Decode_Init() {
	memset(&U, 0, sizeof(U));
	U.RxIndex = RxIndex_Stopped;
}

void RFP_Transport_Decode_SetCallback(void (*cb)(void* data, int length)) {
	U.decoder_cb = cb;
}

void RFP_Transport_Decode_Block(void* data, int length) {
	if (length <= 0) return;
	unsigned int size = length > BUFFERSIZE_MAX ? BUFFERSIZE_MAX : length;
	const uint8_t* arr = data;
	for (unsigned int i = 0; i < size; i++) {
		RFP_Transport_Decode_Byte(arr[i]);
	}
}

void RFP_Transport_Decode_Byte(uint8_t data) {
	if (data == RFP_STX) {
		U.RxIndex = 0;
		return;
	}
	if (U.RxIndex >= 0) {
		if ((unsigned int) U.RxIndex >= sizeof(U.Buffer)) {
			U.RxIndex = RxIndex_Overflow;
			return;
		}
		if (data == RFP_ESC) {
			U.Escape = true;
			return;
		}
		if (U.Escape) {
			data ^= RFP_STUFFING_MASK;
			U.Escape = false;
		}
		U.Buffer[U.RxIndex++] = data;
		if (U.RxIndex > 3) {
			// Len already received
			const uint8_t len = U.Parcel.Len > 128 ? 0 : U.Parcel.Len;
			if (U.RxIndex >= (1+1+1+len+2)) { // CMD+IDX+LEN+PAYLOAD+CRC
				if (U.decoder_crc_checker) {
					if (!U.decoder_crc_checker(U.Buffer, U.RxIndex)) {
						U.RxIndex = RxIndex_BadCRC;
						return;
					}
				}
				if (U.decoder_cb) U.decoder_cb(U.Buffer, U.RxIndex - 2);
				U.RxIndex = RxIndex_Complete;
			}
		}

	}
}

