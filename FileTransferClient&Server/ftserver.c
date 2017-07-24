/*
	Corey Hemphill
	hemphilc@oregonstate.edu
	CS372_400 - Intro. to Computer Networks
	Project 2 - Simple File Transfer Client & Server
	March 12, 2017
	ftserver.c
*/

#include "ftserver.h"


void printError(char* msg) {
	// Print a custom error message
	fprintf(stderr, "%s\n", msg);
	fflush(stderr); // Don't forget to courtesy flush
	exit(1); // Exit the program with code 1 to indicate error
}


int controlConnection(int controlSockfd, int* clientPort, char* command, char* fileName) {
	char inputPayload[MAX_PACKET_LEN]; // Incoming payload
	char outputPayload[MAX_PACKET_LEN]; // Outgoing payload
	char inputHandle[MAX_CMD_LEN + 1]; // Input command handle
	char outputHandle[MAX_CMD_LEN + 1]; // Output command handle
	
	fprintf(stdout, "\nGetting client DATA-PORT...\n");
	receivePacket(controlSockfd, inputHandle, inputPayload); // Attempt to receive the client's port number
	if(strcmp(inputHandle, "DPORT") == 0) {
		*clientPort = atoi(inputPayload);
		fprintf(stdout, "DATA-PORT: %d\n", *clientPort);
	}
	
	fprintf(stdout, "\nGetting client COMMAND and FILENAME...\n");
	receivePacket(controlSockfd, inputHandle, inputPayload); // Attempt to receive client's command and transfer file name
	fprintf(stdout, "\nCOMMAND and FILENAME packet received...\n");
	// Copy the received information into variables and pass by reference
	strcpy(command, inputHandle);
	strcpy(fileName, inputPayload);
	
	if((strcmp(inputHandle, "GET") != 0) && (strcmp(inputHandle, "LIST") != 0)) {
		// We failed to receive a valid command from the client
		fprintf(stdout, "\nFailed to receive the COMMAND transmission from the client...\n");
		strcpy(outputHandle, "ERROR");
		strcpy(outputPayload, "COMMAND Error: enter either -l for LIST or -g for GET");
		transmitPacket(controlSockfd, outputHandle, outputPayload);
		return -1; // Return a -1 to indicate a failure
	}
	else {
		// We successfully received a valid command from the client
		fprintf(stdout, "\nACKing client's request to transmit data...\n");
		strcpy(outputHandle, "OKAY");
		transmitPacket(controlSockfd, outputHandle, "");
		if(DEBUG) { fprintf(stdout, "\nACK transmitted...\n"); }
		return 0; // Return a 0 to indicate a success
	}
}


int dataConnection(int controlSockfd, int dataSockfd, char* command, char* fileName) {
	int ret = 0; // Return status
	int fileCount; // The number of files in the server directory
	char** files; // A list of the files in the server directory
	int i; // For loop counting variable
	
	// Grab the directory's list of file names
	files = getDirectory(".", &fileCount);
	
	// Check to see if the client is requesting a listing of the files in the directory
	if(strcmp(command, "LIST") == 0) {
		fprintf(stdout, "\nTransmitting server directory listing...\n");
		for(i = 0; i < fileCount; i++) {
			transmitPacket(dataSockfd, "FNAME", files[i]);
		}
	}
	// Transmit the requested file to the client
	else if(strcmp(command, "GET") == 0) {
		do {
			bool fExists = false; // Boolean for indicating a file's existence
			FILE* input = fopen(fileName, "r"); // The file descriptor of the input file being read
			char inputBuffer[MAX_PACKET_LEN]; // Input buffer for reading in
			int bytesRead; // A count of the number of bytes that have been read in
		
			// Search all files in the directory to see if there is a match to the given filename
			for(i = 0; i < fileCount; i++) {
				// If there is a file in the directory with the same name of our given file name,
				// then we know that the file exists and we will set our boolean indicator to true
				if(strcmp(fileName, files[i]) == 0) {
					fExists = true;
					break;
				}
			}
			// If the file doesn't exist, send an error message back to the client
			if(!fExists) {
				fprintf(stderr, "\nERROR: The file %s does not exist in the server directory...\n", fileName);
				transmitPacket(controlSockfd, "ERROR", "The specified file does not exist in the server directory...");
				ret = -1; // Return error value
				break;
			}
			// If the input file descriptor is null, send an error message back to the client
			if(input == NULL) {
				fprintf(stderr, "ERROR: File cannot be read...\n");
				transmitPacket(controlSockfd, "ERROR", "Could not read file...");
				ret = -1; // Return error value
				break;
			}
			
			// Everything is good to go!!
			// First, transmit the file name to the client
			transmitPacket(dataSockfd, "FILE", fileName);
			// Now transmit the file to the client
			fprintf(stdout, "\nTransmitting \"%s\" to the client...\n", fileName);
			do {
				// Read some bytes from the input file -- Keep track of how many bytes we've read
				bytesRead = fread(inputBuffer, sizeof(char), MAX_PACKET_LEN - 1, input);
				// Null terminate the buffer to avoid seg fault
				inputBuffer[bytesRead] = '\0';
				// Transmit the data we've just read from the input file to the client
				transmitPacket(dataSockfd, "FILE", inputBuffer);
			} while(bytesRead > 0); // Keep reading from the input file until we've reached the end
			// Make sure that we've actually read from the file and transmitted data
			if(ferror(input)) {
				fprintf(stderr, "There was an error reading from the input file...\n");
				ret = -1;
			}
			// Close the input file
			fclose(input);
		} while(false);
	}
	// The client must have provided an invalid command -- print an error and prompt to try again
	else {
		fprintf(stderr, "ERROR: Command must be \"-g\" for GET or \"-l\" for LIST... Please try again.\n");
		ret = -1;
	}
	// Let the client know we are finished and connection will close
	fprintf(stdout, "\nTerminating the connection with the client...\n");
	transmitPacket(dataSockfd, "DONE", "");
	transmitPacket(controlSockfd, "CLOSE", "");
	
	// Free the memory used by the directory list
	for(i = 0; i < fileCount; i++)
		free(files[i]);
	free(files);
	// Return status
	return ret;
}


void receive(int sockfd, void *buffer, int nBytes){
	int total; // Total received byte count
	int rBytes = 0; // Number of bytes received
	// Attempt to receive all of the bytes
	while(rBytes < nBytes) {
		total = recv(sockfd, (buffer + rBytes), (nBytes - rBytes), 0); // Receive the bytes
		if(total == -1) { printError("\nError receiving..."); } // Something went wrong...
		else { rBytes += total; } // Increment received byte count and continue receiving
	}
}


void receivePacket(int sockfd, char* command, char* buffer) {
	unsigned short packetLen; // Total number of bytes contained within the packet
	unsigned short dataLen; // Total number of bytes of data within the packet
	char cmdBuffer[MAX_CMD_LEN + 1]; // Buffer for storing the received command
	char dataBuffer[MAX_PACKET_LEN]; // Buffer for storing the received data
	// Receive the total length of the packet
	receive(sockfd, &packetLen, sizeof(packetLen));
	if(DEBUG) { fprintf(stdout, "Packet length received..."); }
	packetLen = ntohs(packetLen);
	// Receive the command
	receive(sockfd, cmdBuffer, MAX_CMD_LEN);
	if(DEBUG) { fprintf(stdout, "\nCommand received..."); }
	cmdBuffer[MAX_CMD_LEN] = '\0'; // Null terminate the string to avoid seg fault
	// Copy the received command to the buffer
	if(command != NULL) { strcpy(command, cmdBuffer); }
	// Receive the packet's data bytes
	dataLen = (packetLen - MAX_CMD_LEN - sizeof(packetLen));
	receive(sockfd, dataBuffer, dataLen);
	if(DEBUG) { fprintf(stdout, "\nPacket data received...\n"); }
	dataBuffer[dataLen] = '\0'; // Null terminate the string to avoid seg fault
	// Copy the received data to the buffer
	if(buffer != NULL) { strcpy(buffer, dataBuffer); }
}


void transmit(int sockfd, void *buffer, int nBytes) {
	int total; // Total transmitted byte count
	int sBytes = 0; // Number of bytes transmitted
	// Attempt to transmit all of the bytes
	while(sBytes < nBytes) {
		total = send(sockfd, (buffer + sBytes), (nBytes - sBytes), 0); // Transmit the bytes
		if(total == -1) { printError("Error transmitting..."); } // Something went wrong...
		else { sBytes += total; } // Increment transmitted byte count and continue transmitting
	}
}


void transmitPacket(int sockfd, char* command, char* buffer) {
	unsigned short packetLen; // Total number of bytes contained within the packet
	char cmdBuffer[MAX_CMD_LEN]; // Buffer for storing the transmitting command
	// Transmit the total length of the packet
	packetLen = htons(sizeof(packetLen) + MAX_CMD_LEN + strlen(buffer));
	transmit(sockfd, &packetLen, sizeof(packetLen));
	// Transmit the command
	memset(cmdBuffer, '\0', MAX_CMD_LEN); // Clear the buffer
	strcpy(cmdBuffer, command);
	transmit(sockfd, cmdBuffer, MAX_CMD_LEN);
	// Transmit the packet's data bytes
	transmit(sockfd, buffer, strlen(buffer));
}


char** getDirectory(char* dirName, int* n) {
	DIR *dir = opendir(dirName); // Directory descriptor
	struct dirent *entry; // Directory entry
	struct stat info; // Directory info
	
	// If the directory is empty or null, print error and bail out
	if(dir == NULL) { printError("Error: Could not open directory...\n"); }
	
	char** list; // A list of file names
	list = NULL;
	*n = 0; // Set the number of files to 0 for counting purposes
	// Iterate through the directory while ignoring subdirectories
	while((entry = readdir(dir)) != NULL) {
		stat(entry->d_name, &info);
		// If its a subdirectory, ignore it
		if(S_ISDIR(info.st_mode)) {
			continue;
		}
		// If the list is NULL, malloc the first entry
		if(list == NULL) {
			list = malloc(sizeof(char*));
		}
		// Otherwise, resize the list by 1
		else {
			list = realloc(list, (*n + 1) * sizeof(char*));
		}
		
		// If the list is still NULL, something has gone wrong -- bail out
		if(list == NULL) { printError("Error: Directory list is NULL...\n"); }
		// Create space for the entry
		list[*n] = malloc((strlen(entry->d_name) + 1) * sizeof(char));
		// If the entry slot is NULL, something has gone wrong -- bail out
		if(list == NULL) { printError("Error: Directory list entry slot is NULL...\n"); }
		// Enter the entry into the list
		strcpy(list[*n], entry->d_name);
		// Add one to the count
		(*n)++;
	}
	// Close the directory
	closedir(dir);
	// Return the list of file names
	return list;
}


void sigintHandler(int signal) {
	int ret; // Return status
	struct sigaction interrupt; // Interupt handler
	
	fprintf(stdout, "\nTerminating ftserver...\n");
	interrupt.sa_handler = SIG_DFL; // Return handler to default signal handling
	
	ret = sigaction(SIGINT, &interrupt, 0); // Handle the signal
	if(ret == -1) { printError("Error handling interrupt signal"); }
	
	ret = raise(SIGINT); // Raise the signal
	if(ret == -1) { printError("Error raising interrupt signal"); }
}


void initServer(int portNum) {
	int ret; // Return status
	int sockfd; // Socket descriptor
	
	struct sigaction interrupt;	// Interupt handler
	interrupt.sa_handler = &sigintHandler; // Register the custom handler function
	interrupt.sa_flags = 0;
	sigemptyset(&interrupt.sa_mask);
	ret = sigaction(SIGINT, &interrupt, 0);
	if(ret == -1) {	printError("Error registering signal handler");	}
	
	struct sockaddr_in address; // Server IP address
	address.sin_family = AF_INET; // Use IPv4
	address.sin_port = htons(portNum); // Assign port number
	address.sin_addr.s_addr = INADDR_ANY; // Use Local host
	
	// Initialize the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) { printError("Server socket initialization failure");	}
	
	// Bind the socket/port/address
	ret = bind(sockfd, (struct sockaddr*) &address, sizeof(address));
	if(ret == -1) {	printError("Socket binding failure"); }
	
	// Start listening for client connections
	ret = listen(sockfd, MAX_PENDING);
	if(ret == -1) { printError("Listening failure"); }
	
	// We can now allow clients to connect to the server via the specified port number
	
	// Run until we receive a signal interrupt
	while(true) {
		fprintf(stdout, "\n-------------------------------------------------------------------------------------");
		fprintf(stdout, "\nFTP server is actively waiting on port %d...\n", portNum);
		struct sockaddr_in clientIP; // Client's address info
		char *clientAddress; // Client's IP address
		int clientPortNum; // Client's port number
		
		char commandHandle[MAX_CMD_LEN]; // Command buffer
		char fileName[MAX_PACKET_LEN]; // Filename of the client's file
		
		int control; // Control socket descriptor
		int data;	// Data socket descriptor
		
		socklen_t addressLength = sizeof(struct sockaddr_in); // Length of the client's address
		control = accept(sockfd, (struct sockaddr *) &clientIP, &addressLength ); // Accept the client connection
		if(control == -1) { printError("Failure to accept the client connection"); }
		
		clientAddress = inet_ntoa(clientIP.sin_addr); // Convert address to readable string
		fprintf(stdout, "\nThe FTP control connection has been initialized with client address \"%s\"\n", clientAddress);
		
		// Initiate the FTP control connection
		ret = controlConnection(control, &clientPortNum, commandHandle, fileName);
		
		if(ret != -1) {
			int count = 0; // Counting variable for tracking the number of connection attempts
			// Initialize data connection socket
			data = socket(AF_INET, SOCK_STREAM, 0);
			if(data == -1) { printError("Data socket initialization failure..."); }
			
			clientIP.sin_port = htons(clientPortNum); // Assign port number
			
			// Attempt to initialize FTP data connection with the client
			do { ret = connect(data, (struct sockaddr *) &clientIP, sizeof(clientIP)); }
			while((count < MAX_TRYS) && (ret == -1)); // Continue attempting to connect if failed
			if(ret == -1) { printError("Failed to create FTP connection with client..."); }
			else { fprintf(stdout, "\nThe FTP data connection has been initialized with client address \"%s\"\n", clientAddress); }
			
			// Use FTP connection for transferring data
			dataConnection(control, data, commandHandle, fileName);
			// Wait for ACK...
			receivePacket(control, NULL, NULL);
			
			// Close the connection
			ret = close(data);
			if(ret == -1) { printError("Failed to close FTP data connection..."); }
			else { 
				fprintf(stdout, "\nThe FTP connection has been successfully closed with client address \"%s\"\n", clientAddress);
				fprintf(stdout, "-------------------------------------------------------------------------------------\n");
			}
		}
	}
}

