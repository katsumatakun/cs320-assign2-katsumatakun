CC = gcc
# CFLAGS = -Wall
@ = pageTable.c
all: pageTable alloc_test

pageTable: pageTable.c
	$(CC) -o pageTable pageTable.c

alloc_test: allocation_test.c
	$(CC) -o alloc_test allocation_test.c

clean:
	rm -f pageTable alloc_test
