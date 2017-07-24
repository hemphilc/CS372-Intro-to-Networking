/*
	Corey Hemphill
	hemphilc@oregonstate.edu
	CS372_400 - Intro. to Computer Networks
	Project 2 - Simple File Transfer Client & Server
	March 12, 2017
	ftserver.h
*/

#ifndef FTSERVER
#define FTSERVER

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT_MIN 1025			// Minimum port number
#define PORT_MAX 65535			// Maximum port number
#define MAX_PACKET_LEN 513 		// Maximum packet size
#define MAX_CMD_LEN 8	 		// Maximum handle length
#define MAX_PENDING 5 			// Maximum number of pending connection requests
#define MAX_TRYS 10 			// Maximum number of connections

#define DEBUG 0

/* Define Boolean Operators */
typedef int bool;
enum {
	false,
	true
};

////////////////////Functions////////////////////

/* Prints an error message and exits the program
	param:  msg	the message string to be printed to console
	pre:    none
	post:   an error message is printed and the program is exited
	ret:    integer value of 1 to indicate error
*/
void printError(char* msg);


/* Establishes communication with client via a control connection
	param:  controlSockfd	control connection socket descriptor
	param:  clientPort	client port number for data transfer
	param:  command	command string buffer
	param:  fileName	the name of the file to be transferred
	pre:    An FTP control connection has been initialized
	post:   command has been received and set
	post:   clientPort has been received and set
	post:   fileName has been received and set
	ret:    0 for success, -1 for failure
*/
int controlConnection(int controlSockfd, int* clientPort, char* command, char* fileName);


/* Transmits file data over the FTP data connection
	param:  controlSockfd	control connection socket descriptor
	param:  dataSockfd	data connection socket descriptor
	param:  command	command string buffer
	param:  fileName	the name of the file to be transferred
	pre:    An FTP control connection has been initialized
	pre:    Socket descriptors have connections established
	post:   the specified file has been transferred
	ret:    0 for success, -1 for failure
*/
int dataConnection(int controlSockfd, int dataSockfd, char* command, char* fileName);


/* Receive all of the bytes being transmitted
	param:  sockfd	socket descriptor
	param:  buffer	buffer to store the received bytes
	param:  nBytes	the number of bytes received
	pre:    there must be a connection initiated on the socket
	pre:    the buffer must be big enough to hold all of the bytes
	post:   the buffer contains all of the received bytes
	ret:    none
*/
void receive(int sockfd, void *buffer, int nBytes);


/* Receives a single packet
	param:  sockfd	socket descriptor
	param:  command	buffer for holding data tag
	param:  data	buffer containing the transmitted bytes
	pre:    there must be a connection initiated on the socket
	pre:    the buffer must be big enough to hold all of the bytes
	post:   the buffer contains all of the received bytes
	post:   command buffer contains the data tag
	ret:    none
*/
void receivePacket(int sockfd, char* command, char* buffer);


/* Transmits all of the bytes being to the client
	param:  sockfd	socket descriptor
	param:  buffer	buffer to store the transmitting bytes
	param:  nBytes	the number of bytes to transmit
	pre:    there must be a connection initiated on the socket
	pre:    the buffer must be big enough to hold all of the bytes
	post:   the buffer contains all of the transmitting bytes
	ret:    none
*/
void transmit(int sockfd, void *buffer, int nBytes);


/* Transmits a single packet
	param:  sockfd	socket descriptor
	param:  command	buffer for holding data tag
	param:  buffer	buffer containing the transmitted bytes
	pre:    there must be a connection initiated on the socket
	pre:    the buffer must be big enough to hold all of the bytes
	post:   the buffer contains all of the transmitting bytes
	post:   command buffer contains the data tag
	ret:    none
*/
void transmitPacket(int sockfd, char* command, char* buffer);


/* Gets the contents of a directory
	param:  dirName	the name of the directory
	param:  n	the number of files in the directory
	pre:    none
	post:   n is the number of files in the directory
	ret:    a list of file names
*/
char** getDirectory(char* dirName, int* n);


/* Terminates the program when interrupt signal is received
	param:  signal   the interupt signal number
	pre:    none
	post:   the program has exited normally via a signal interrupt
	ret:    none
*/
void sigintHandler(int signal);


/* Initializes a server to wait on a port for a client for ftp file transfer
	param:	portNum	the server port number
	pre:    portNum is greater than 1025 and less than 65535
	post:   an ftp server has been initiated until a signal interrupt is received
	ret:    none
*/
void initServer(int portNum);

/////////////////////////////////////////////////

 #endif // FTSERVER
