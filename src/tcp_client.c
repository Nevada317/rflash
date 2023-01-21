// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 2048

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#include "tcp_client.h"

static int sockfd = -1;


pthread_t thread_reader = 0;

void (*rx_cb)(void* data, int length);

void PrintBuffer(char* tag, void* data, int size) {
	int left = size;
	unsigned char* ptr = data;
	if (left <= 0) {
		printf(" Tried to print packet with length %d and tag *%s*!\n", left, tag);
		return;
	}
	if (tag) {
		printf(" (%s) Packet with length %d:\n", tag, left);
	} else {
		printf(" Packet with length %d:\n", left);
	}
	uint8_t slen = 0;
	uint32_t ii = 0;
	while (left) {
		if (!slen) {
			printf("%08x:  ", ii);
		}
		printf(" %02X", ptr[ii++]);
		if (slen++ == 15) {
			slen = 0;
			printf("\n");
		}
		left--;
	}
	if (slen) {
		printf("\n");
	}

}

static void* async_reader(void* arg) {
	char buff[MAX];
	while (1) {
		if (*((volatile int*)arg) == -1) break;
		int length = read(*((volatile int*)arg), buff, sizeof(buff));
		if (length) {
			PrintBuffer("Rx", buff, length);
			if (rx_cb) rx_cb(buff, length);
		}
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
		if (!length) {
			printf("Server Exit due to empty read...\n");
			break;
		}
	}
	printf("Closing...\n");
	return 0;
}

bool connect_tcp_qualified(char* address, int port, void (*cb)(void* data, int length)) {
	struct sockaddr_in servaddr;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		return false;
	} else {
		printf("Socket successfully created..\n");
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(address);
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		close(sockfd);
		sockfd = -1;
		return false;
	} else {
		printf("connected to the server..\n");
	}

	rx_cb = cb;

	pthread_create(&thread_reader, 0, async_reader, &sockfd);

	return true;
}

void disconnect_tcp() {
	if (sockfd == -1) return;
	volatile int old_fd = sockfd;
	pthread_cancel(thread_reader);
	pthread_join(thread_reader, NULL);
	sockfd = -1;
	close(old_fd);
}

void server_send(void* data, int length) {
	PrintBuffer("Tx", data, length);
	if (sockfd) {
		write(sockfd, data, length);
	}
}
