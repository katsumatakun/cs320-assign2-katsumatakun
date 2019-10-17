CC = gcc
CFLAGS = -Wall
@ = pageTable.c
all: pageTable

pageTable: pageTable.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f pageTable
