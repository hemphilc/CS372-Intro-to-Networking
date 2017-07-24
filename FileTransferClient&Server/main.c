/*
	Corey Hemphill
	hemphilc@oregonstate.edu
	CS372_400 - Intro. to Computer Networks
	Project 2 - Simple File Transfer Client & Server
	March 12, 2017
	main.c
*/

#include "ftserver.h"


int main(int argc, char *argv[]) {
	int portNumber; // Server port number
	
	// Check for valid number of arguments
	if(argc == 2) {
		char tmp;
		int check = sscanf(argv[1], "%d %c", &portNumber, &tmp);
		if(check != 1) {
			// The user did not provide a valid integer port number, print error message
			printError("\nPlease provide a valid port number...\nUsage: ./ftserver <SERVER-PORT>\n");
		}
		if(DEBUG) {
			printf("port: %d\n", portNumber);
		}
		// Verify port number range
		if(portNumber < PORT_MIN || portNumber > PORT_MAX) {
			printError("\nValid port number range is between 1025 and 65535\nUsage: ./ftserver <SERVER-PORT>\n");
		}
	}
	else {
		// There are not enough/too many arguments, print error message
		printError("\nUsage: ./ftserver <SERVER-PORT>\n");
	}

	// Initialize the FTP server
	initServer(portNumber);
	
	return 0;
}