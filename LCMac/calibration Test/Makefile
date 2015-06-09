CC=gcc
CFLAGS=-g -Wall -lm
all:	program
cali1: calibrationTest.c picomms.c mapping.c turning.c linkedList.c 
	$(CC) $(CFLAGS) -o calibrationTest calibrationTest.c picomms.c mapping.c turning.c linkedList.c -lm

cali2: middleTest.c picomms.c mapping.c turning.c linkedList.c
	$(CC) $(CFLAGS) -o calibrationTest middleTest.c picomms.c mapping.c turning.c linkedList.c  -lm