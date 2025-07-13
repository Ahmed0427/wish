CC=gcc
CFLAGS=-Wall -Wextra -Werror
EXEC=wish

all:
	$(CC) $(CFLAGS) -o $(EXEC) main.c
