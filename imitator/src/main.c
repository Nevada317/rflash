#include <stdio.h>
#include "tcp_server.h"

void my_cb(void* data, int length) {
	printf("Rx cb: %d @ %p\n", length, data);
}

int main(int argc, char *argv[]) {
	server_start(8080, &my_cb);
	return 0;
}
