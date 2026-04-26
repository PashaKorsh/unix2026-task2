CC = gcc
CFLAGS = -Wall -D_POSIX_C_SOURCE=200809L

all: locker

locker: main.c
	$(CC) $(CFLAGS) -o locker main.c

clean:
	rm -f locker stat.txt result.txt testfile testfile.lck

.PHONY: all clean