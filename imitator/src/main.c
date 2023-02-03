#include <stdio.h>
#include "tcp_server.h"
#include "periodic.h"
#include "rfp.h"

#include <stdlib.h>

periodic_timer_t* poll_timer;

void my_cb(void* data, int length) {
	printf("Rx cb: %d @ %p\n", length, data);
	PERIODIC_TriggerTimer(poll_timer);
	// server_send(data, length);
	// server_send("test\n", 5);
}

void test_cb(void*) {
	printf("Tick...\n");
	rfp_buffer_t report = {0,};
	rfp_flexbuffer_t* flex = RFP_CreateParcel(RFP_CMD_REPORT, 0, &report);
	server_send(flex->Data, flex->Length);
	free(flex);
}

int main(int argc, char *argv[]) {
	poll_timer = PERIODIC_CreateTimer(test_cb, 0, 5000);
	server_start(8080, &my_cb);
	PERIODIC_DestroyTimer(poll_timer);
	return 0;
}
