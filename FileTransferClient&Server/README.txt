Corey Hemphill
hemphilc@oregonstate.edu
CS372_400 - Intro. to Computer Networks
Project 2 - Simple File Transfer Client & Server
March 12, 2017
README.txt


This is a simple FTP system in which a client sends a command to the server to
either obtain a directory listing or request a file from the server's directory
listing to be transferred to the client. This system is comprised of two seperate 
programs: an FTP server and an FTP client. There are five files total:

	- ftclient.py (written in python)
	- ftserver.c (written in C)
	- ftserver.h (written in C)
	- main.c (written in C)
	- Makefile


To execute:
	1. Place the ftclient.py file into a DIFFERENT directory than the ftserver.c, 
		ftserver.h, main.c, and Makefile files. The user should navigate to these 
		working directories via command line in seperate terminal windows. In order 
		to run ftclient.py, it must be given executable permissions. To give 
		executable permission, enter the following command into the command line
		from the directory where ftclient.py resides:

		chmod a+x ftclient.py


	2. Start ftserver on Host A. Open a new terminal and navigate to the
		working directory where ftserver files are located. Before the user can 
		start ftserver, the program must be compiled by the Makefile via command: 

		make

		Once the program has been compiled, enter "./ftserver <SERVER-PORT>" into 
		the command line (without quotes and angle brackets) where SERVER-PORT is 
		the server-side port number.

		Example: ./ftserver 54132
		
		Note: user should use a non-standard portnumber in the range [1025, 65535].

		The server will now wait on the SERVER-PORT for a client connection.


	3. Start ftclient on Host B. This program does not need to be compiled. Open a 
		new terminal and navigate to the working directory where ftclient file is located.
		Enter "python ftclient.py <SERVER-HOST> <SERVER-PORT> <COMMAND> <FILENAME> <DATA-PORT>" 
		into the command line (without quotes and angle brackets) where SERVER-HOST is the 
		hostname that the server is waiting on, SERVER-PORT is the port number that the server 
		is waiting on, COMMAND is the user command, FILENAME is the name of the file to be 
		transferred, and DATA-PORT is the client's port number for data transfer.
		
		The SERVER-HOST entered must be the hostname of the host in which ftserver has 
		been started. The SERVER-PORT must be the port number that ftserver is waiting on.
		The COMMAND must either be -l for LIST or -g for GET. The FILENAME must be the 
		name of a file in the server's directory listing and is only required when using
		the GET command. The DATA-PORT is the port number that the ftclient is receiving
		data on.

		Example: python ftclient.py flip2 54132 -l 54133
		
		Example: python ftclient.py flip2 54132 -g example.txt 54133

		Note: user should use a non-standard portnumber in the range [1025, 65535].
		
		Note: SERVER-PORT and DATA-PORT cannot be the same port number.
		
		Note: to query hostname via command line, simply enter "hostname" (without quotes).
		
		The ftserver on Host A will now respond to ftclient's request.


	4. ftclient will terminate once the user's command has been completed by the ftserver.
		To terminate ftserver, send a SIGINT using ctrl-C via command line.


These programs have been tested on the following machines:
	- flip1.engr.oregonstate.edu
	- flip2.engr.oregonstate.edu
	- flip3.engr.oregonstate.edu

	
References:
	- http://beej.us/guide/bgnet/
	- http://unix.stackexchange.com/questions/168794/how-to-get-the-list-of-ports-which-are-free-in-a-unix-server
	- http://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
	- http://www.csl.mtu.edu/cs4411.ck/www/NOTES/signal/install.html
	- http://www.microhowto.info/howto/convert_an_ip_address_to_a_human_readable_string_in_c.html
	- https://www.tutorialspoint.com/c_standard_library/c_function_ferror.htm
	- http://stackoverflow.com/questions/11243841/ftp-server-in-c
	- http://stackoverflow.com/questions/21525957/implementing-ftp-server-client-in-c
	- https://github.com/fedackb/ftp-server
	- http://stackoverflow.com/questions/12991334/members-of-dirent-structure
	- http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
	- https://www.tutorialspoint.com/python/python_variable_types.htm
	- http://stackoverflow.com/questions/12693606/reason-for-globals-in-python
	- https://www.programiz.com/python-programming/methods/built-in/globals
	- https://docs.python.org/2/library/socket.html
	- http://stackoverflow.com/questions/18718709/using-struct-pack-in-python
	- https://docs.python.org/3.0/library/struct.html

