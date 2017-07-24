#!/usr/bin/python

# Corey Hemphill
# hemphilc@oregonstate.edu
# CS372_400 Intro. to Computer Networks
# Project 2 - Simple File Transfer Client & Server
# March 12, 2017
# ftclient.py

import os
import re
import sys
from socket import (
    socket,
    gethostbyname,
    AF_INET,
    SOCK_STREAM,
    SOL_SOCKET,
    SO_REUSEADDR
)
from struct import (
	pack,
	unpack
)

PORT_MIN = 1025 # Minimum port number
PORT_MAX = 65535 # Maximum port number
MAX_CMD_LEN = 8 # Maximum command handle length
MAX_PENDING = 5 # Maximum number of pending connection requests

DEBUG = 1 # Debug flag

# Prints error message and usage text to the console
#	param:  msg 	error message to be printed to console
#	pre:    none
#	post:   error message is printed to console
#	post:   usage text is printed to the console
#	post:   the program has been terminated
#	ret:    none
def printUsageErr(msg = ""):
	# Print error, print usage, and exit the program
	print msg
	print "\nUsage: python ftclient.py <SERVER-HOST> <SERVER-PORT> <COMMAND> <FILENAME> <DATA-PORT>"
	print "\nUsage Notes:"
	print '\tCOMMAND: "-l" for directory list | "-g" for get a file'
	print "\tFILENAME: required with command get (-g)"
	print "\tSERVER-PORT | DATA-PORT: range is [1025, 65535]"
	print "\tSERVER-PORT and DATA-PORT cannot be the same"
	print "\nUsage Examples:"
	print "\tpython ftclient.py LocalHost 6251 -l 6252"
	print "\tpython ftclient.py LocalHost 6251 -g example.txt 6252\n"
	print "-------------------------------------------------------------------------------------\n"
	exit(1) # Indicate failure


# Prints error message to the console
#	param:  msg 	error message to be printed to console
#	pre:    none
#	post:   error message is printed to console
#	post:   the program has been terminated
#	ret:    none
def printErr(msg = ""):
	# Print error and exit the program
	print msg
	print "-------------------------------------------------------------------------------------\n"
	exit(1) # Indicate error


# Initializes a client to contact a server for an ftp file transfer
#	param:	SERVER_HOST	global variable for server hostname
#	param:	SERVER_PORT	global variable for server port number
#	pre:    SERVER_PORT is greater than 1024 and less than 65535
#	post:   an ftp client has been initiated until a signal interrupt is received
#	ret:    none
def initClient():
	try:
		# Create control socket for FTP connection
		controlSockfd = socket(AF_INET, SOCK_STREAM, 0)
	except Exception as error:
		print "Control socket creation failure... Exiting"
		printErr(error)
	
	try:
		# Attempt to initiate connection with server
		controlSockfd.connect((SERVER_HOST, SERVER_PORT))
	except Exception as error:
		print "Connection creation failure... Exiting"
		printErr(error)
	
	# We have successfully created the control socket and control connection
	print "\nThe FTP control connection has been initialized with server address \"{0}\"\n".format(SERVER_HOST, SERVER_PORT)
	
	# Initiate the control connection
	ret = controlConnection(controlSockfd)

	if ret != -1:
		try:
			# Create data socket for FTP connection
			sockfd = socket(AF_INET, SOCK_STREAM, 0)
		except Exception as error:
			print "Socket failure... Exiting"
			printErr(error)
	
		try:
			# Bind the socket and data port number
			sockfd.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
			sockfd.bind(("", DATA_PORT))
		except Exception as error:
			print "Binding failure... Exiting"
			printErr(error)
	
		try:
			# Begin listening for the server response
			sockfd.listen(MAX_PENDING)
		except Exception as error:
			print "Listening failure... Exiting"
			printErr(error)

		try:
			# Accept the data connection
			dataSockfd = sockfd.accept()[0]
		except Exception as error:
			print "Acceptance failure... Exiting"
			printErr(error)
		
		# We have successfully created the data socket and data connection
		print "\nThe FTP data connection has been initialized with server address \"{0}\"\n".format(SERVER_HOST)
		
		# Use FTP connection for transferring data
		dataConnection(controlSockfd, dataSockfd)
		
		while True:
			cmd, input = receivePacket(controlSockfd)
			# Server indicated an error occurred
			if cmd == "ERROR":
				print "ERROR: " + input
			# Server sent close connection command, bail out and kill the connection
			if cmd == "CLOSE":
				break
				
	try:
		# Attempt to close the connection with the server
		controlSockfd.close()
	except Exception as error:
		print "Closing failure... Exiting"
		printErr(error)

	# The connection has been closed successfully
	print "\nThe FTP connection has been successfully closed with server address \"{0}\"".format(SERVER_HOST)


# Establishes communication with client via a control connection
#	param:  cSockfd	control connection socket descriptor
#	param:  COMMAND	global vairable for command string buffer
#	param:  DATA_PORT	global variable for data port number
#	pre:    An FTP control connection has been initialized
#	post:   command has been received and set
#	post:   clientPort has been received and set
#	post:   fileName has been received and set
#	ret:    0 for success, -1 for failure
def controlConnection(cSockfd):
	cmd = "DPORT" # Let server know we are sending the data port number
	output = str(DATA_PORT) # Convert the data port number to string
	
	# Attempt to send the data to the server
	print "Transmitting the DATA-PORT to the server to establish a data connection...\n"
	transmitPacket(cSockfd, cmd, output)

	# Attempt to send the user command to the server
	print "Transmitting the COMMAND to the server...\n"
	# Check if the user wants the directory listening
	if COMMAND == "-l":
		cmd = "LIST"
	# Check if the user is indicating a file to be transferred
	elif COMMAND == "-g":
		output = FILENAME
		cmd = "GET"
	# Transmit the user's request to the server
	transmitPacket(cSockfd, cmd, output)
	
	# Wait for the server's acknowledgement
	print "Waiting for server to ACK the COMMAND...\n"
	cmd, payload = receivePacket(cSockfd)
	print "Server ACK received..."

	# Check to see if there was an error on the server
	if cmd == "ERROR":
		print "ERROR: " + payload
		return -1 # Failure
	
	return 0 # Success


# Transmits file data over the FTP data connection
#	param:  cSockfd	control connection socket descriptor
#	param:  dSockfd	data connection socket descriptor
#	param:  COMMAND	global variable for command string buffer
#	param:  FILENAME	global variable for the name of the file to be transmitted
#	pre:    An FTP control connection has been initialized
#	pre:    Socket descriptors have connections established
#	post:   the specified file has been transferred
#	ret:    0 for success, -1 for failure
def dataConnection(cSockfd, dSockfd):
	ret = 0 # Initialize return value as successful
	# Receive the initial packet from the server
	cmd, payload = receivePacket(dSockfd)
	# Check to see if server is transmitting a file
	if cmd == "FILE":
		FILENAME = payload
		# Check to see if the file already exists
		if os.path.exists(FILENAME):
			print "\"{0}\" already exists in the client directory...".format(FILENAME)
			ret = -1 # Indicate failure
		else:
			with open(FILENAME, "w") as file:
				# Proceed to write the transmitted data
				while cmd != "DONE":
					cmd, payload = receivePacket(dSockfd)
					file.write(payload)
			print "File transfer completed successfully -- \"{0}\" has been received...".format(FILENAME)
	# Check to see if the server is transmitting a directory listing
	elif cmd == "FNAME":
		print "Directory listing on server \"{0}\"\n".format(SERVER_HOST, SERVER_PORT)
		# Print the received file names until the server indicates it is finished transmitting
		while cmd != "DONE":
			print "\t- " + payload
			cmd, payload = receivePacket(dSockfd)
	else:
		ret = -1

	# Send ACK to server indicating all packets have been received
	transmitPacket(cSockfd, "ACK", "")
	return ret


# Receive all of the bytes being transmitted
#	param:  sockfd	socket descriptor
#	param:  nBytes	the number of bytes received
#	pre:    there must be a connection initiated on the socket
#	pre:    the buffer must be big enough to hold all of the bytes
#	post:   none 
#	ret:    the data received
def receive(sockfd, nBytes):
	# Receive the transmitted data from the server
	payload = ""
	while len(payload) < nBytes:
		try:
			# Add the bytes received to the total payload
			payload += sockfd.recv(nBytes - len(payload))
		except Exception as error:
			print "Receiving failure... Exiting"
			printErr(error)

	return payload


# Receives a single packet
#	param:  sockfd	socket descriptor
#	pre:    there must be a connection initiated on the socket
#	pre:    the buffer must be big enough to hold all of the bytes
#	post:   the buffer contains all of the received bytes
#	post:   command buffer contains the data tag
#	ret:    none
def receivePacket(sockfd):
	# Receive packet length from server
	packetLen = unpack(">H", receive(sockfd, 2))[0]
	# Receive the command
	cmd = receive(sockfd, MAX_CMD_LEN).rstrip("\0")
	# Receive the transmitted data
	payload = receive(sockfd, packetLen - MAX_CMD_LEN - 2)
	return cmd, payload


# Creats and transmits a single packet
#	param:  sockfd	socket descriptor
#	param:  command	buffer for holding data tag
#	param:  buffer	buffer containing the transmitted bytes
#	pre:    there must be a connection initiated on the socket
#	pre:    the buffer must be big enough to hold all of the bytes
#	post:   the buffer contains all of the transmitting bytes
#	post:   command buffer contains the data tag
#	ret:    none
def transmitPacket(sockfd, command = "", buffer = ""):
	# Build the packet
	packetLen = MAX_CMD_LEN + len(buffer) + 2
	packet = pack(">H", packetLen)
	packet += command.ljust(MAX_CMD_LEN, "\0")
	packet += buffer

	try:
		# Transmit the packet
		sockfd.sendall(packet)
	except Exception as error:
		print "Transmission failure... Exiting"
		printErr(error)


# Main function that validates command line arguments and initiates the FTP client
#	param:  SERVER_HOST	global variable for the server hostname
#	param:  SERVER_PORT	global variable for the server port number
#	param:  COMMAND	global variable for command string buffer
#	param:  FILENAME	global variable for the name of the file to be transmitted
#	param:  DATA_PORT	global variable for the data port number
#	ret:    0 for success, 1 for failure
def main():
	# Check to ensure we are receiving a valid number of arguments
	if len(sys.argv) not in (5, 6):
		# If not, print error message, print usage text, and exit the program
		printUsageErr("\nError: Please provide valid parameters...")

	# Assign all command line arguments as globals for easy passing
	global SERVER_HOST
	global SERVER_PORT
	global COMMAND
	global FILENAME
	global DATA_PORT
	SERVER_HOST = gethostbyname(sys.argv[1])
	SERVER_PORT = sys.argv[2]
	COMMAND = sys.argv[3]
	# If there are 5 arguments, then filename should be the fourth argument
	# If there are only 4 arguments, then filename should not exist as an argument
	FILENAME = sys.argv[4] if len(sys.argv) == 6 else None
	# If there are 5 arguments, then data port number should be the fifth argument
	# If there are only 4 arguments, then data port number should be the fourth argument
	DATA_PORT = sys.argv[5] if len(sys.argv) == 6 else sys.argv[4]

	print "\n-------------------------------------------------------------------------------------"
	
	print "SERVER-HOST: %s" % SERVER_HOST
	print "SERVER-PORT: %s" % SERVER_PORT
	print "COMMAND: %s" % COMMAND
	print "FILENAME: %s" % FILENAME
	print "DATA-PORT: %s" % DATA_PORT
		
	# Validate all user inputs...
	# Check to see if the user command is valid
	if COMMAND not in ("-l", "-g"):
		printUsageErr("\nCOMMAND is invalid...")

	# Check to see there is a filename with user "get" command
	if FILENAME is None and COMMAND == "-g":
		printUsageErr("\nYou must provide a FILENAME when using COMMAND get file (-g)...")

	# Check to see if the server port number is an integer value
	if re.match("^[0-9]+$", SERVER_PORT) is None:
		printUsageErr("\nSERVER-PORT must be an integer...")
	else:
		SERVER_PORT = int(SERVER_PORT)

	# Check to see if the server port number is in the valid range
	if int(SERVER_PORT) < PORT_MIN or int(SERVER_PORT) > PORT_MAX:
		printUsageErr("\nSERVER-PORT is out of range...")

	# Check to see if the data port number is an integer value
	if re.match("^[0-9]+$", DATA_PORT) is None:
		printUsageErr("\nDATA-PORT must be an integer...")
	else:
		DATA_PORT = int(DATA_PORT)

	# Check to see if the server port number is in the valid range
	if int(DATA_PORT) < PORT_MIN or int(DATA_PORT) > PORT_MAX:
		printUsageErr("\nDATA-PORT is out of range...")

	# Check to see if the server port number and data port number are the same
	if SERVER_PORT == DATA_PORT:
		printUsageErr("\nSERVER-PORT & DATA-PORT cannot be the same...")
		
	# All user input has been successfully validated
	# We can now initialize the FTP client
	initClient()
	
	print "-------------------------------------------------------------------------------------\n"
	exit(0) # Exit indicating success


# Call the main function
if __name__ == "__main__":
	main()

