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

// Function designed for chat between client and server.
void func(int connfd)
{
	char buff[MAX];
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);

		// read the message from client and copy it in buffer
		int length = read(connfd, buff, sizeof(buff));
		// print buffer which contains the client contents
		printf("From client (%d bytes): %s\n", length, buff);
		// copy server message in the buffer
		// while ((buff[n++] = getchar()) != '\n')
			// ;

		// and send that buffer to client
		// write(connfd, buff, sizeof(buff));

		// if msg contains "Exit" then server exit and chat ended.
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
	}
	close(connfd);
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
			func(connfd);
		}
		break;
	}

	// After chatting close the socket
	close(sockfd);
	return 0;
}
