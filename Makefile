CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -std=gnu99 -g

all: server client

server: server.o
	$(CC) $(CFLAGS) $^ -o $@

client: client.o
	$(CC) $(CFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o client server
