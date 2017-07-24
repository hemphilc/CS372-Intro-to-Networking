#!/usr/bin/python

# Corey Hemphill
# hemphilc@oregonstate.edu
# CS372_400 Intro. to Computer Networks
# Project 1 - Simple Chat Client & Server
# February 12, 2017
# chatserve.py

import socket
import sys

MESSAGE_LENGTH = 500

# Establishes a chat session session with a chat client
#	param:  connSocket	the connect socket descriptor
#	param:  clientHandle	the client's chat handle
#	param:  handle	the server's chat handle
#	pre:    socket descriptor is valid and connection is established
#	post:   chat is initialized with client and messages can be exchanged
#	ret:    none
def initChat(connSocket, clientHandle, handle):
	while 1:
		# Receive the massage from the chat client.
		# Receive message + 1 to take into account the null termination
		# being sent from the other end (even though python doesn't need
		# it, do it to clear the buffer).
		message = connSocket.recv(MESSAGE_LENGTH + 1)[0:-1]

		# Print the received message to the console with prompt prepended
		print '{}> {}'.format(clientHandle, message)
		buffer = '' # Clear the chat buffer
		# Prompt the user to type their message. Attempt again if the message
		# is less than or equal to 0 chars/bytes or greater than 500 chars/bytes
		while len(buffer) == 0 or len(buffer) > MESSAGE_LENGTH:
			buffer = raw_input('{}> '.format(handle))

		# The user wants to quit the chat, print connection closed, break
		# out of the function, and start waiting for new connections 
		if buffer == '\quit':
			print "\nConnection closed"
			print "Waiting for new connection"
			break

		# Otherwise, we send our message and wait for the client to respond
		connSocket.send(buffer)

# Exchanges handles with client through the new connection
#	param:  connSocket	the connect socket descriptor
#	param:  handle	the server's chat handle
#	pre:    socket descriptor is valid and connection is established
#	post:   client has received the server's handle, server received client's
#	ret:    the client's handle
def HandShake(connSocket, handle):
	"""HandShake -- Exchanges handles with client connection"""
	# Receive the client's chat handle through the newly established connection
	clientHandle = connSocket.recv(1024)
	# Send a response with the server's chat handle
	connSocket.send(handle)
	return clientHandle

# Main function that creates a socket and waits for connection
#	pre:    socket descriptor is valid and connection is established
#	post:   client has received the server's handle, server received client's
#	ret:    the client's handle
def main():
	# Check to ensure we are receiving a valid number of arguments
	if len(sys.argv) != 2:
		# If not, print error, print usage, and exit the program
		print "\nPlease provide the server's port number..."
		print "Usage: python chatserver.py <server port number>\n"
		exit(1)

	HOST = '' # We will use the local hosts name, but for now, leave it blank
	PORT = int(sys.argv[1]) # We will use the port number provided via command line
	# Creat the socket and bind. Begin listening for connections on the given port
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind((HOST, PORT)) 
	s.listen(1)

	print "Server initialized... Waiting for connections..."
	while 1:
		# Accept the incoming connection
		connSocket, IPaddress = s.accept()
		print "Established connection with address {}\n".format(IPaddress)
		# Initialize the chat sessiono
		initChat(connSocket, HandShake(connSocket, 'ChatServer'), 'ChatServer')
		# Chat session has been closed, time to close the socket
		connSocket.close()

# Call the main function
if __name__ == "__main__":
	main()

