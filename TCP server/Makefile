CC=g++
CFLAGS= -g -Wall -Werror

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client.o -c client.c
	$(CC) $(CFLAGS) -o client client.o

server: server.c
	$(CC) $(CFLAGS) -o server.o -c server.c
	$(CC) $(CFLAGS) -o server server.o -lpthread

clean:
	rm -f client server *.o

zip:
	zip -r cis3210_ass1_$(USER).zip client.c server.c README Makefile
