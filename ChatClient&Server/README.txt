Corey Hemphill
hemphilc@oregonstate.edu
CS372_400 - Intro. to Computer Networks
Project 1 - Simple Chat Client & Server
February 12, 2017
README.txt


This is a simple chat system that works for one pair of users to take turns 
sending messages back and forth. This system is comprised of two seperate 
programs: a chat server and a chat client. There are two files:

	- chatserve.py (written in python)
	- chatclient.c (written in C)

To execute:
	1. Place both chatserve.py and chatclient.c into the same working directory.
		The user should navigate to this working directory via command line.
		In order to run chatserve.py, it must be given executable permissions.
		To give permission, enter the following command into the command line:

		chmod a+x chatserve.py


	2. Start chatserve.py on Host A. This program does not need to be compiled.
		Enter "python chatserve.py <portnum>" into the command line (without 
		quotes) where portnum is a port number of the user's choice.

		Example: python chatserve.py 54132

		User should use a non-standard portnumber such as 14141, 54323, etc. If 
		successful, user will see the following message: "Server initialized... 
		Waiting for connections..." Host A is now waiting for a client request.

		FYI: to query hostname via command line, simply enter "hostname" (without
		quotes)


	3. Start chatclient.c on Host B. Open a new terminal and navigate to the
		working directory where chatclient.c is located. Before the user can 
		start chatclient, the program must be compiled using the following command: 

		gcc chatclient.c -o chatclient

		Once the program has been compiled, enter "chatclient <hostname> <portnum>"
		into the command line (without quotes) where hostname is the name of Host A
		(i.e. flip2.engr.oregonstate.edu) and portnum is the same port number given
		to chatserve on Host A. The host name entered must be the name of the host 
		in which chatserve has been started. The port number must be the port number 
		chatserve was given by the user.

		Example: chatclient flip1.engr.oregonstate.edu 54132

		The user will first be prompted to enter a chat handle of length 10 or less.
		The chatclient may now send the initial message to chatserve, establishing
		the connection. Once initial message is received by chatserve, chatserve
		may reply. Both chatserve and chatclient may alternate sending and
		receiving messages until either user decides to quit.


	4. To quit the chat and close the connection, either party can enter "\quit" 
		into the command line (without quotes). If chatclient quits, the program
		terminated, and it must be reinitialized via the command line. If chatserve 
		quits, it continues to run waiting for other connections. To terminate 
		chatserve, send a SIGINT using ctrl-C.


These programs have been tested on the following machines:

	- flip1.engr.oregonstate.edu
	- flip2.engr.oregonstate.edu
	- flip3.engr.oregonstate.edu

	
References:
	- http://stackoverflow.com/questions/23401147/what-is-the-difference-between-struct-addrinfo-and-struct-sockaddr
	- https://docs.python.org/2/library/socket.html
	- http://codingnights.com/coding-fully-tested-python-chat-server-using-sockets-part-1/
	- http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
	- http://stackoverflow.com/questions/5956516/getaddrinfo-and-ipv6
	- http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
