#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "chatroom.h"

#define MAX 1024					 // max buffer size
#define PORT 6789					 // server port number
#define MAX_USERS 50				 // max number of users
static unsigned int users_count = 0; // number of registered users

static user_info_t *listOfUsers[MAX_USERS] = {0}; // list of users

/* different from skeleton*/
/* Add user to userList */
int user_add(user_info_t *user);
/* Delete user from userList */
void user_delete(int sockfd);
/* Get user name from userList */
char *get_username(int sockfd);
/* Get user sockfd by name */
int get_sockfd(char *name);

/*TODO*/
/* Check if name already exist*/
int check_name_dup(char *name)
{
	for (int i = 0; i < users_count; i++)
	{
		if (strcmp(name, listOfUsers[i]->username) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/*TODO*/
/* Add user to userList */
int user_add(user_info_t *user)
{
	if (users_count == MAX_USERS)
	{
		printf("sorry the system is full, please try again later\n");
		return 0;
	}

	/***************************/
	/* add the user to the list */
	/**************************/

	listOfUsers[users_count] = malloc(sizeof(user_info_t));
	*listOfUsers[users_count] = *user;
	users_count++;
	return 1;

	/**************************/
}

/*TODO*/
/* Delete user from userList */
void user_delete(int ss)
{
	for (int k = 0; k < users_count; k++)
	{
		/***************************/
		/* delete the user from the list */
		/**************************/
		if (listOfUsers[k]->sockfd == ss)
		{
			for (int j = k; j < users_count - 1; j++)
			{
				listOfUsers[j] = listOfUsers[j + 1];
			}
		}
		/**************************/
	}
	users_count--;
}

/*TODO*/
/* Get user name from userList */
char *get_username(int ss)
{
	int i;
	static char uname[MAX];
	/*******************************************/
	/* Get the user name by the user's sock fd */
	/*******************************************/

	for (i = 0; i < users_count; i++)
	{
		if (listOfUsers[i]->sockfd == ss)
		{
			bzero(uname, sizeof(uname));
			strcpy(uname, listOfUsers[i]->username);
		}
	}

	/*******************************************/
	printf("get user name: %s\n", uname);
	return uname;
}

/*TODO*/
/* Get user sockfd by name */
int get_sockfd(char *name)
{
	int i;
	int sock;
	/*******************************************/
	/* Get the user sockfd by the user name */
	/*******************************************/
	for (i = 0; i < users_count; i++)
	{
		if (strcmp(listOfUsers[i]->username, name) == 0)
		{
			sock = listOfUsers[i]->sockfd;
			return sock;
		}
	}
	/*******************************************/
	sock = -1;
	return sock;
}

int main()
{
	fd_set master;	 // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int fdmax;		 // maximum file descriptor number

	int listener;  // listening socket descriptor
	int newfd;	   // newly accept()ed socket descriptor
	int addr_size; // length of client addr
	struct sockaddr_in server_addr, client_addr;

	char buffer[MAX]; // buffer for client data
	int nbytes;

	int yes = 1; // for setsockopt() SO_REUSEADDR, below
	int i, j, rv;

	FD_ZERO(&master); // clear the master and temp sets
	FD_ZERO(&read_fds);

	/*TODO*/
	/**********************************************************/
	/*create the listener socket and bind it with server_addr*/
	/**********************************************************/

	// sget us a socket and bind it
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1)
	{
		printf("Socket creation failed...\n");
		exit(1);
	}
	else
		printf("Socket successfully created..\n");

	// lose the pesky "address already in use" error message
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	bzero(&server_addr, sizeof(server_addr));

	// assign IP, PORT
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	int bindFlag = bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (bindFlag != 0)
	{
		printf("Socket bind failed...\n");
		exit(2);
	}
	else
		printf("Socket successfully binded..\n");

	/**********************************************************/

	// Now server is ready to listen and verification
	if ((listen(listener, MAX_USERS)) != 0)
	{
		printf("Listen failed...\n");
		exit(3);
	}
	else
		printf("Server listening..\n");

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// main loop
	for (;;)
	{
		/*TODO*/
		/***************************************/
		/* use select function to get read_fds */
		/**************************************/
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
		{
			perror("select");
			exit(4);
		}
		/**************************************/

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++)
		{

			if (FD_ISSET(i, &read_fds))
			{ // we got one!!
				if (i == listener)
				{
					/*TODO*/
					/**************************/
					/* we are the listener and we need to handle new connections from clients */
					/****************************/

					// handle new connections
					addr_size = sizeof(client_addr);
					newfd = accept(listener, (struct sockaddr *)&client_addr, &addr_size);
					if (newfd == -1)
					{
						perror("accept");
					}
					else
					{
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax)
						{ // keep track of the max
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
							   "socket %d\n",
							   inet_ntoa(client_addr.sin_addr), newfd);
					}

					/****************************/

					// send welcome message
					bzero(buffer, sizeof(buffer));
					strcpy(buffer, "Welcome to the chat room!\nPlease enter a nickname.\n");
					if (send(newfd, buffer, sizeof(buffer), 0) == -1)
						perror("send");
				}
				else
				{
					// handle data from a client
					bzero(buffer, sizeof(buffer));
					if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0)
					{
						// got error or connection closed by client
						if (nbytes == 0)
						{
							// connection closed
							printf("selectserver: socket %d hung up\n", i);

							/*difference from skeleton*/
							/*********************************/
							/* Broadcast the leave message to the other users in the group*/
							/**********************************/
							bzero(buffer, sizeof(buffer));
							strcpy(buffer, get_username(i));
							strcat(buffer, " has left the chatroom \n");
							for (int k = 0; k < users_count; k++)
							{
								if (listOfUsers[k]->sockfd == i)
								{
									continue;
								}
								if (send(listOfUsers[k]->sockfd, buffer, sizeof(buffer), 0) <= 0)
								{
									perror("Send Failed");
								}
							}
							user_delete(i);
							/**************************/
						}
						else
						{
							perror("recv");
						}

						close(i);			// bye!
						FD_CLR(i, &master); // remove from master set
					}
					else
					{
						// we got some data from a client
						if (strncmp(buffer, "REGISTER", 8) == 0)
						{
							printf("Got register message\n");

							/*TODO*/
							/********************************/
							/* Get the user name and add the user to the userlist*/
							/**********************************/

							char name[C_NAME_LEN + 1];
							bzero(name, sizeof(name));
							char *token = strtok(buffer, " ");
							token = strtok(NULL, "\n");

							// if name is duplicated
							if (check_name_dup(token) == 1)
							{
								// re-enter the name
								bzero(buffer, sizeof(buffer));
								strcpy(buffer, "The nickname has been taken. Please enter a new nickname.\n");
								if (send(newfd, buffer, sizeof(buffer), 0) == -1)
									perror("send");
							}			
							else
							{
								strcpy(name, token);

								user_info_t new_registration;
								new_registration.sockfd = i;
								strcpy(new_registration.username, name);

								int is_success = user_add(&new_registration);

								/**********************************/

								// broadcast the welcome message
								bzero(buffer, sizeof(buffer));
								strcpy(buffer, "Welcome ");
								strcat(buffer, name);
								strcat(buffer, " to join the chat room!\n");

								/*TODO*/
								/*****************************/
								/* Broadcast the welcome message*/
								/*****************************/

								if (is_success)
								{
									for (int k = 0; k < users_count; k++)
									{
										int sock_tmp = listOfUsers[k]->sockfd;
										if (send(sock_tmp, buffer, sizeof(buffer), 0) <= 0)
										{
											perror("Error: Send Failed");
										}
									}
								}
								else
								{
									bzero(buffer, sizeof(buffer));
									strcpy(buffer, "Sorry the system is full, please try again later\n");
									if (send(i, buffer, sizeof(buffer), 0) <= 0)
									{
										perror("Error: Send Failed");
									}
								}

								/*****************************/
							}
						}
						else if (strncmp(buffer, "EXIT", 4) == 0)
						{
							printf("Got exit message. Removing user from system\n");
							// send leave message to the other members
							bzero(buffer, sizeof(buffer));
							strcpy(buffer, get_username(i));
							strcat(buffer, " has left the chatroom \n");

							/*TODO*/
							/*********************************/
							/* Broadcast the leave message to the other users in the group*/
							/**********************************/
							for (int k = 0; k < users_count; k++)
							{
								if (listOfUsers[k]->sockfd == i)
								{
									continue;
								}
								if (send(listOfUsers[k]->sockfd, buffer, sizeof(buffer), 0) <= 0)
								{
									perror("Send Failed");
								}
							}

							/**********************************/

							/*TODO*/
							/***********************************/
							/* close the socket, delete the user and remove the socket from the fd array*/
							/***********************************/

							close(i);
							FD_CLR(i, &master); // remove from master set
							user_delete(i);
							/**********************************/
						}
						else if (strncmp(buffer, "WHO", 3) == 0)
						{
							// concatenate all the user names except the sender into a char array
							printf("Got WHO message from client.\n");
							char ToClient[MAX];
							bzero(ToClient, sizeof(ToClient));

							/*TODO*/
							/***************************************/
							/* Concatenate all the user names into the tab-separated char ToClient and send it to the requesting client*/
							/***************************************/

							for (int k = 0; k < users_count; k++)
							{
								if (listOfUsers[k]->sockfd != i)
								{
									strcat(ToClient, get_username(listOfUsers[k]->sockfd));
									strcat(ToClient, "\t");
								}
							}
							strcat(ToClient, "\n");

							if (send(i, ToClient, sizeof(ToClient), 0) <= 0)
							{
								perror("Error: Failed to send WHO response");
							}

							/*************************************/
						}
						else if (strncmp(buffer, "#", 1) == 0)
						{
							// send direct message
							// get send user name:
							printf("Got direct message.\n");
							// get which client send the message
							char sendname[MAX];
							// get dest sock
							int destsock;
							// get the message
							char msg[MAX];

							/*TODO*/
							/**************************************/
							/* Get the source name xx, and the target sockfd*/
							/*************************************/
							strcpy(sendname, get_username(i));

							char *token = strtok(buffer, "#");
							// token = name
							token = strtok(token, ":");
							destsock = get_sockfd(token);

							// the receiving client does not exist
							if (destsock == -1)
							{
								bzero(msg, sizeof(msg));
								strcpy(msg, "Sorry, the user you typed doesn't exist. Please check your input format or type \"WHO\" to see whether the user has left. \n");
								if (send(i, msg, sizeof(msg), 0) == -1)
								{
									perror("send");
								}
							}
							else
							{
								// token = message body
								token = strtok(NULL, "\n");
								strcpy(msg, token);
								strcat(msg, "\n");
								/*************************************/

								// concatenate the message in the form "xx to you: msg"
								char sendmsg[MAX * 2];
								bzero(sendmsg, sizeof(sendmsg));
								strcpy(sendmsg, sendname);
								strcat(sendmsg, " to you: ");
								strcat(sendmsg, msg);
								if (send(destsock, sendmsg, sizeof(sendmsg), 0) == -1)
								{
									perror("send");
								}
							}
						}
						else
						{
							printf("Got broadcast message from user\n");

							/*TODO*/
							/*********************************************/
							/* Broadcast the message to all users except the one who sent the message*/
							/*********************************************/

							char msg[MAX];
							char *token = strtok(buffer, "\n");
							strcpy(msg, token);
							strcat(msg, "\n");

							for (int k = 0; k < users_count; k++)
							{
								if (get_sockfd(listOfUsers[k]->username) == i)
								{
									continue;
								}
								if (send(listOfUsers[k]->sockfd, msg, sizeof(msg), 0) <= 0)
								{
									perror("Error: Send Failed");
								}
							}
							/*********************************************/
						}
					} // end handle data from client
				}	  // end got new incoming connection
			}
		} // end looping through file descriptors

	} // end for(;;)

	return 0;
}
