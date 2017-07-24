/*
	Corey Hemphill
	hemphilc@oregonstate.edu
	CS372_400 - Intro. to Computer Networks
	Project 1 - Simple Chat Client & Server
	February 12, 2017
	chatclient.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>

#define HANDLE_LENGTH 11
#define MESSAGE_LENGTH 501
#define DEBUG 0

/* Prints an error message and exits the program
	param:  name	the name of the function that failed
	param:  code	the error code to print
	pre:    none
	post:   an error message is printed and the program is exited
	ret:    none
*/
void printError(char* name, int code) {
	// Print a custom error message
	fprintf(stderr, "%s Error: %s. Please try again...\n", name, gai_strerror(code));
	fflush(stderr); // Don't forget to courtesy flush
	exit(1); // Exit the program with code 1 to indicate error
}

/* Gets the address structure to which we need to connect
	param:  hostName	the host name we wish to connect to
	param:  portNumber	the host's listening port number
	pre:    hostName cannot be null
	pre:    portNumber cannot be null
	post:   an address structure has been created
	ret:    a new address structure
*/
struct addrinfo* getAddress(char* hostName, char* portNumber) {
	assert(hostName != NULL && portNumber != NULL);
	struct addrinfo hints; // Hints for specifying socket address structure
	struct addrinfo *result = NULL; // The actual result address structure we return
	
	// Clear and initialize the hints addrinfo struct
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // Use TCP socket
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE; // Use the local host's IP address
	
	// Grab the address information struct using host name and port number
	int status = getaddrinfo(hostName, portNumber, &hints, &result);
	if(status == 0)
		return result; // Status is good, return the new address struct
	else
		printError("getAddress", status); // Status has returned an error
}

/* Creates a socket using an address structure
    param:	result	an address structure to a host we want to connect to
    pre:    result cannot be null
    post:   a socket has been created using the given address structure
    ret:    a new socket descriptor
*/
int getSocket(struct addrinfo *result) {
	assert(result != NULL);
	// Attempt to create the new socket using the provided address structure
	int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(sockfd == -1)
		printError("getSocket", sockfd); // Socket creation has failed
	return sockfd; // Return the new socket descriptor
}

/* Establishes a connection between two hosts
	param:	result	an address structure to a host we want to connect to
	param:	sockfd	a socket descriptor
	pre:    result cannot be null
	pre:    sockfd must be a valid socket descriptor
	post:   a connection has been created between two hosts
	ret:    none
*/
void getConnection(struct addrinfo *result, int sockfd) {
	assert(result != NULL);
	// Attempt to establish a connection
	int status = connect(sockfd, result->ai_addr, result->ai_addrlen);
	if(status == -1)
		printError("getConnection", status); // Socket creation has failed
}

/* Initializes a chat connection between two hosts for sending messages
	param:	clientName	the client's chat handle
	param:	serverName	the server's chat handle
	param:	sockfd	a socket descriptor
	pre:    clientName cannot be null
	pre:    serverName cannot be null
	pre:    sockfd must be a valid socket descriptor
	post:   a chat connection has been created between two hosts for sending messages
	ret:    none
*/
void doChat(char* clientName, char* serverName, int sockfd) {
	assert(clientName != NULL && serverName != NULL);
	int recvBytes;
	int sentBytes = 0;
	char chatSend[MESSAGE_LENGTH]; // Buffer for message being sent
	char chatRecv[MESSAGE_LENGTH]; // Buffer for message being received
	
	// Get the user's initial message to be sent
	memset(chatSend, 0, sizeof(chatSend)); // Zero out the send buffer
	fgets(chatSend, MESSAGE_LENGTH, stdin);
	
	while(1) {
		// Print the prompt
		printf("%s> ", clientName);
		
		// Get the user's message to be sent
		memset(chatSend, 0, sizeof(chatSend)); // Zero out the send buffer
		fgets(chatSend, MESSAGE_LENGTH, stdin);
		
		// Check to see if the user wants to quit the chat
		if(strcmp(chatSend, "\\quit\n") == 0) {
			break; // User wants to quit -- break out of the while loop
		}
		// Otherwise, send the message
		else {
			sentBytes = send(sockfd, chatSend, MESSAGE_LENGTH, 0);
			// Check to see if something went wrong
			if(sentBytes == -1) {
				fprintf(stderr, "Error: Message could not be sent...\n");
				fflush(stderr); // Don't forget to courtesy flush
				exit(1); // Exit the program with code 1 to indicate error
			}
		}

		// Attempt to receive a message
		memset(chatRecv, 0, sizeof(chatRecv)); // Zero out the receive buffer
		recvBytes = recv(sockfd, chatRecv, MESSAGE_LENGTH, 0);
		if(recvBytes < 0) {
			fprintf(stderr, "Error: Message could not be received...\n");
			fflush(stderr); // Don't forget to courtesy flush
			exit(1); // Exit the program with code 1 to indicate error
		}
		else if(recvBytes > 0) {
			printf("%s> %s\n", serverName, chatRecv);
		}
		else {
			printf("The server has dropped the connection.\n");
			break;
		}
	}
	// Close the socket/connection
	printf("\nClosing connection...\n");
	close(sockfd);
}

int main(int argc, char *argv[]) {
	char* host; // Server host name
	char* port; // Server port number
	char clientHandle[HANDLE_LENGTH]; // Client's chat handle
	char serverHandle[HANDLE_LENGTH]; // Server's chat handle
	
	// Check for valid number of arguments
	if(argc == 3) {
		host = argv[1];
		port = argv[2];
		if(DEBUG) {
			printf("host: %s\n", host);
			printf("port: %s\n", port);
		}
	}
	else {
		// There are not enough valid arguments, print error message
		fprintf(stderr, "\nPlease provide valid host name and port number...\nUsage: ./chatclient <host name> <port number>\n");
		fflush(stderr);
		exit(1);
	}

	// Get the client's chat handle
	printf("Enter a chat handle of length 10 or less: ");
	scanf("%s", clientHandle);
	printf("\n");

	// Obtain the client's IP address
	struct addrinfo *result = getAddress(host, port);
	
	// Create a socket for the connection
	int sockfd = getSocket(result);
	
	// Create the connection using the address and socket
	getConnection(result, sockfd);
	
	// Do handshake
	int s = send(sockfd, clientHandle, strlen(clientHandle), 0); // Send the client's handle
	int r = recv(sockfd, serverHandle, HANDLE_LENGTH, 0); // Receive the server's handle
	
	if(DEBUG) {
		printf("send: %d\n", s);
		printf("recv: %d\n", r);
	}
	
	// Interesting issue... You must null terminate the server's handle because its being sent from
	// a python based program. Python does not require null termination to strings. Therefore, there
	// will be no null terminator appended to this string when it is received. Null termination must
	// be done manually here, otherwise strange behavior and extra random characters will be seen 
	// as part of the handle.
	serverHandle[10] = '\0';
	
	// Start the chat session
	doChat(clientHandle, serverHandle, sockfd);
	
	freeaddrinfo(result); // Free the address struct
	
	return 0;
}
