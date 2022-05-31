#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "chatroom.h"

#define MAX 1024  // max buffer size
#define PORT 6789 // port number

static int sockfd;

void generate_menu()
{
	printf("Hello dear user please select one of the following options:\n");
	printf("EXIT\t-\t Send exit message to server - unregister ourselves from server\n");
	printf("WHO\t-\t Send WHO message to the server - get the list of current users including ourselves\n");
	printf("#<user>: <msg>\t-\t Send <MSG>> message to the server for <user>\n");
	printf("Or input messages sending to everyone in the chatroom.\n");
}

// function for receiver thread
void *recv_server_msg_handler()
{
	/********************************/
	/* receive message from the server and display on the screen*/
	/**********************************/

	char recv_buffer[MAX];
	bzero(&recv_buffer, sizeof(recv_buffer));

	while (1)
	{
		int received = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
		if (received > 0)
		{
			printf("%s", recv_buffer);
		}
		else if (received == 0)
		{
			printf("Nothing is received. \n");
			printf("The server terminates \n");
			exit(0);
			break;
		}
		else
		{
			perror("Fail to receive message from server");
		}
	}

	pthread_exit(NULL);
}

int main()
{
	int n;
	int nbytes;
	struct sockaddr_in server_addr, client_addr;
	char buffer[MAX];

	// TODO
	/******************************************************/
	/* create the client socket and connect to the server */
	/******************************************************/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created...\n");

	bzero(&server_addr, sizeof(server_addr));

	// server socket IP, PORT
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(PORT);

	// connect the client socket to the server socket
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
	{
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the server\n");

	generate_menu();
	/****************************************************************************************/

	// receive welcome message
	bzero(buffer, sizeof(buffer));
	if (nbytes = recv(sockfd, buffer, sizeof(buffer), 0) == -1)
	{
		perror("recv");
	}
	printf("%s", buffer);

	// TODO
	/*************************************/
	/* Let the user receive the prompt from the server and enter the nickname to register. */
	/* Note that we concatenate "REGISTER" before the name to notify the server it is the register message*/
	/*******************************************/

	// length = 24 characters + new line = 25
	char name[C_NAME_LEN + 1];
	bzero(name, sizeof(name));

	int start = 0;
	while ((name[start++] = getchar()) != '\n')
		;

	// loop to check the length of the name
	while (1)
	{
		// start will increment one last time
		// index will become 25 instead of 24
		// need to minus 1
		if (start - 1 > C_NAME_LEN)
		{
			printf("The name is too long with %d characters \n", start - 1);
			printf("Enter a new name with a maximum of %d characters only. \n ", C_NAME_LEN);
			bzero(name, sizeof(name));
			start = 0;
			while ((name[start++] = getchar()) != '\n')
				;
		}
		else
		{
			break;
		}
	}

	bzero(buffer, sizeof(buffer));
	strcpy(buffer, "REGISTER ");
	strcat(buffer, name);

	if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
	{
		perror("Error: Registration failed");
	}

	/*******************************************/

	/* different than skeletion*/
	// ideal receive welcome message "welcome xx to joint the chatroom."
	bzero(buffer, sizeof(buffer));
	if (recv(sockfd, buffer, sizeof(buffer), 0) == -1)
	{
		perror("recv");
	}
	printf("%s", buffer);

	// if the system is full
	while (strcmp(buffer, "Sorry the system is full, please try again later\n") == 0)
	{
		bzero(buffer, sizeof(buffer));
		strcpy(buffer, "REGISTER ");

		char name[C_NAME_LEN];
		bzero(name, sizeof(name));
		int start = 0;
		while ((name[start++] = getchar()) != '\n')
			;

		strcat(buffer, name);

		if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
		{
			perror("Error: Registration failed");
		}

		bzero(buffer, sizeof(buffer));
		if (recv(sockfd, buffer, sizeof(buffer), 0) == -1)
		{
			perror("recv");
		}
		printf("%s", buffer);
	}
	// if nickname taken
	while (strcmp(buffer, "The nickname has been taken. Please enter a new nickname.\n") == 0)
	{
		bzero(buffer, sizeof(buffer));
		strcpy(buffer, "REGISTER ");

		char name[C_NAME_LEN];
		bzero(name, sizeof(name));
		int start = 0;
		while ((name[start++] = getchar()) != '\n')
			;

		strcat(buffer, name);

		if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
		{
			perror("Error: Registration failed");
		}

		bzero(buffer, sizeof(buffer));
		if (recv(sockfd, buffer, sizeof(buffer), 0) == -1)
		{
			perror("recv");
		}
		printf("%s", buffer);
	}

	/***************************************************************/

	// TODO
	/*****************************************************/
	/* Create a thread to receive message from the server*/
	/*****************************************************/


	pthread_t recv_thread;
	int iret_recv = pthread_create(&recv_thread, NULL, &recv_server_msg_handler, NULL);
	if (iret_recv < 0)
	{
		perror("Thread creation failed");
	}

	/*******************************************/

	// chat with the server
	for (;;)
	{
		bzero(buffer, sizeof(buffer));
		n = 0;
		while ((buffer[n++] = getchar()) != '\n')
			;

		if ((strncmp(buffer, "EXIT", 4)) == 0)
		{
			printf("Client Exit...\n");

			// TODO
			/********************************************/
			/* Send exit message to the server and exit */
			/********************************************/

			bzero(buffer, sizeof(buffer));
			strcpy(buffer, "EXIT");

			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				perror("Exit Failed.");
			}
			close(sockfd);

			return 0;
			/*******************************************/
		}
		else if (strncmp(buffer, "WHO", 3) == 0)
		{
			printf("Getting user list, pls hold on...\n");
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				puts("Sending MSG_WHO failed");
				exit(1);
			}
			printf("If you want to send a message to one of the users, pls send with the format: '#username:message'\n");
		}
		else if (strncmp(buffer, "#", 1) == 0)
		{
			// different from skeleton
			while (1)
			{
				// n will increment one last time
				// need to minus 1
				if (n > MAX)
				{
					printf("The message is too long with %d characters (excluded the newline character) \n", n - 1);
					printf("Enter prompt with a maximum of %d characters only (excluded the newline character). \n ", MAX - 1);
					bzero(buffer, sizeof(buffer));
					n = 0;
					while ((buffer[n++] = getchar()) != '\n')
						;
				}
				else
				{
					break;
				}
			}

			// If the user want to send a direct message to another user, e.g., aa wants to send direct message "Hello" to bb, aa needs to input "#bb:Hello"
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				printf("Sending direct message failed...");
				exit(1);
			}
		}
		else
		{
			// TODO
			/*************************************/
			/* Sending broadcast message. The send message should be of the format "username: message"*/
			/**************************************/

			char send_message[MAX];
			bzero(&send_message, sizeof(send_message));
			strncpy(send_message, name, strlen(name) - 1);
			strcat(send_message, ": ");
			int prefix = strlen(send_message);
			
			while (1)
			{

				if (n > MAX - prefix)
				{
					printf("The message is too long with %d characters (excluded the newline character) \n", n - 1);
					printf("Enter prompt with a maximum of %d characters only (excluded the newline character). \n ", MAX - prefix -1);
					bzero(buffer, sizeof(buffer));
					n = 0;
					while ((buffer[n++] = getchar()) != '\n')
						;
				}
				else
				{
					break;
				}
			}



			strcat(send_message, buffer);

			if (send(sockfd, send_message, sizeof(send_message), 0) < 0)
			{
				printf("Sending broadcast message failed...");
				exit(1);
			}

			/*******************************************/
		}
	}
	return 0;
}
