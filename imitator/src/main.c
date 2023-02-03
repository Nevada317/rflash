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
	rfp_task_status_t Status;
} Workers[2] = {0,};

rfp_buffer_t DataBuffer[2] = {0,};

void my_cb(void* data, int length) {
	const rfp_transport_rx_t* D = data;
	const uint8_t W = D->Idx & 1;
	const bool match = (Workers[W].ActiveTask == D->Idx);
	switch (D->Cmd) {
		case RFP_CMD_RESET:
			memset(Workers, 0, sizeof(Workers));
			break;
		case RFP_CMD_POLL:
			PERIODIC_TriggerTimer(poll_timer);
			break;
		case RFP_CMD_Add0:
			Workers[W].Status = RFP_Task_GotAdd0;
			Workers[W].ActiveTask = D->Idx;
			break;
		case RFP_CMD_Add1:
			if (match) {
				Workers[W].Status = RFP_Task_GotAdd1;
			}
			break;
		case RFP_CMD_Add2:
			if (match) {
				Workers[W].Status = RFP_Task_Finished;
			}
			break;
		default:
			break;
	}
	// printf("Rx cb: %d @ %p\n", length, data);
	PrintBuffer("RxL", data, length);
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

int main(int, char **) {
	poll_timer = PERIODIC_CreateTimer(test_cb, 0, 5000);
	RFP_Transport_Decode_SetCallback(&my_cb);
	server_start(8080, &RFP_Transport_Decode_Block);
	PERIODIC_DestroyTimer(poll_timer);
	return 0;
}
