CC=gcc
CFLAGS=-lcurses
STATSFILE=stats.txt

all: window.c
	@$(CC) $(CFLAGS) -o typescore window.c
	@printf "0\n0\n0\n0" > $(STATSFILE)
