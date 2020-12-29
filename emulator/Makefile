CFLAGS=-std=c11 -Wall -Wextra -Iz80 -g -O0
LDFLAGS=-lutil

all: emulator

emulator: main.o z80/Z80.o
	gcc -o $@ $^ ${LDFLAGS}

.PHONY: clean
clean:
	rm -f *.o emulator

# vim:ts=8:sts=8:sw=8:noexpandtab
