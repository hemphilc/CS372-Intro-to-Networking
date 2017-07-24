#	Corey Hemphill
#	hemphilc@oregonstate.edu
#	CS372_400 - Intro. to Computer Networks
#	Project 2 - Simple File Transfer Client & Server
#	March 12, 2017
#	Makefile

default: ftserver

ftserver.o: ftserver.h ftserver.c
	gcc -g -c ftserver.c

ftserver: ftserver.o main.c
	gcc -g -o ftserver ftserver.o main.c
	
clean:
	rm ftserver.o
	
cleanall: clean
	rm ftserver