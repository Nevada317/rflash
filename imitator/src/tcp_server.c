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
#define PORT 8080

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

static volatile bool StopThreads = false;

pthread_t thread_reader = 0;

void* async_reader(void* arg) {
	char buff[MAX];
	int fd = *((int*)arg);
	while (1) {
		int length = read(fd, buff, sizeof(buff));
		if (length) {
			printf("From client (%d bytes): %s\n", length, buff);
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
	close(fd);
	return 0;
}

// Driver function
int server_start()
{
	int sockfd, connfd;
	unsigned int len;
	struct sockaddr_in servaddr, cli;

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
	servaddr.sin_port = htons(PORT);

	int true_int = 1;
	setsockopt(sockfd, AF_INET, SO_REUSEADDR, &true_int, sizeof(true_int));

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
