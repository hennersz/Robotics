CC=gcc
CFLAGS=-g -Wall -lm
all:	program
program: mazefollower.c picomms.c mapping.c wallfollower.c
	$(CC) $(CFLAGS) -o mazefollower mazefollower.c picomms.c mapping.c  wallfollower.c -lm