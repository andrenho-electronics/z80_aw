CFLAGS=-std=c11 -Wall -Wextra
LDFLAGS=-lutil

all: emulator

emulator: main.o
	gcc -o $@ $^ ${LDFLAGS}

.PHONY: clean
clean:
	rm -f *.o emulator

# vim:ts=8:sts=8:sw=8:noexpandtab
