CC=gcc
CFLAGS=-g -Wall -lm
all:	program
program: calibrationTest.c picomms.c mapping.c turning.c linkedList.c 
	$(CC) $(CFLAGS) -o calibrationTest calibrationTest.c picomms.c mapping.c turning.c linkedList.c -lm