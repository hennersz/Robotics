CC=gcc
CFLAGS=-c -Wall
SOURCES=mainCode.c phase1.c phase1.map.c phase1.move.c recordPosition.c picomms.c basicMovement.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mazeTask

all:	$(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE):	$(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ -lm

.c.o:
	$(CC) $(CFLAGS) $< -o $@
