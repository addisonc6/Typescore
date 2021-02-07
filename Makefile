CC=gcc
CFLAGS=-lcurses

all: window.c
	$(CC) $(CFLAGS) -o typescore window.c
