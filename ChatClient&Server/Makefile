default: chatclient

chatclient.o: chatclient.h chatclient.c
	gcc -g -c chatclient.c

chatclient: chatclient.o main.c
	gcc -g -o chatclient chatclient.o main.c
	
clean:
	rm chatclient.o
	
cleanall: clean
	rm chatclient