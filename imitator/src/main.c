#include <stdio.h>
#include "tcp_server.h"
#include "periodic.h"
#include "rfp.h"
#include "rfp_transport_decoder.h"

#include <stdlib.h>
#include <string.h>

periodic_timer_t* poll_timer;

struct {
	uint8_t ActiveTask;
	uint8_t Status;
} Workers[2] = {0,};

rfp_buffer_t DataBuffer[2] = {0,};

void my_cb(void* data, int length) {
	printf("Rx cb: %d @ %p\n", length, data);
	PERIODIC_TriggerTimer(poll_timer);
	// server_send(data, length);
	// server_send("test\n", 5);
}

void test_cb(void*) {
	printf("Tick...\n");
	rfp_buffer_t report = {0,};
	memcpy(report.Payload, Workers, sizeof(Workers));
	report.PayloadSize = sizeof(Workers);
	rfp_flexbuffer_t* flex = RFP_CreateParcel(RFP_CMD_REPORT, 0, &report);
	server_send(flex->Data, flex->Length);
	free(flex);
}

int main(int argc, char *argv[]) {
	poll_timer = PERIODIC_CreateTimer(test_cb, 0, 5000);
	RFP_Transport_Decode_SetCallback(&my_cb);
	server_start(8080, &RFP_Transport_Decode_Block);
	PERIODIC_DestroyTimer(poll_timer);
	return 0;
}
