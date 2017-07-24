#	Corey Hemphill
#	hemphilc@oregonstate.edu
#	CS372_400 - Intro. to Computer Networks
#	Project 1 - Simple Chat Client & Server
#	March 12, 2017
#	Makefile

default: chatclient

chatclient.o: chatclient.h chatclient.c
	gcc -g -c chatclient.c

chatclient: chatclient.o main.c
	gcc -g -o chatclient chatclient.o main.c
	
clean:
	rm chatclient.o
	
cleanall: clean
	rm chatclient