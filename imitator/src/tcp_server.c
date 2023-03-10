// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 2048

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

pthread_t thread_reader = 0;

void (*rx_cb)(void* data, int length);

volatile int response_fd = 0;

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

void* async_reader(void* arg) {
	char buff[MAX];
	int fd = *((int*)arg);
	response_fd = fd;
	while (1) {
		int length = read(fd, buff, sizeof(buff));
		if (length > 0) {
			PrintBuffer("Rx", buff, length);
			if (rx_cb) rx_cb(buff, length);
		}
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
		if (length <= 0) {
			printf("Server Exit due to empty read (length = %d)...\n", length);
			break;
		}
	}
	printf("Closing...\n");
	response_fd = 0;
	close(fd);
	return 0;
}

// Driver function
int server_start(int port, void (*cb)(void* data, int length)) {
	int sockfd, connfd;
	unsigned int len;
	struct sockaddr_in servaddr, cli;

	rx_cb = cb;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		return 1;
	}
	else {
		printf("Socket successfully created\n");
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	int true_int = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true_int, sizeof(true_int));

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		return 1;
	}
	else {
		printf("Socket successfully binded..\n");
	}

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		return 1;
	}
	else {
		printf("Server listening..\n");
	}

	while (1) {
		len = sizeof(cli);
		// Accept the data packet from client and verification
		connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
		if (connfd < 0) {
			printf("server accept failed...\n");
			return 1;
		}
		else {
			printf("server accept the client...\n");
			// Function for chatting between client and server

			if (thread_reader) {
				pthread_cancel(thread_reader);
				pthread_join(thread_reader, NULL);
				thread_reader = 0;
			}
			pthread_create(&thread_reader, 0, async_reader, &connfd);
		}
	}

	pthread_cancel(thread_reader);
	pthread_join(thread_reader, NULL);

	// After chatting close the socket
	close(sockfd);
	return 0;
}

void server_send(void* data, int length) {
	PrintBuffer("Tx", data, length);
	if (response_fd) {
		write(response_fd, data, length);
	}
}
