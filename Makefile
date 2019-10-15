CC = gcc
CFLAGS = -Wall

all: mmu

%: %.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f mmu

