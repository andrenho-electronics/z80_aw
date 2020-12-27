CFLAGS=-std=c11 -Wall -Wextra

emulator: main.o
	gcc -o $@ $^ ${LIBS}

.PHONY: clean
clean:
	rm -f *.o emulator

# vim:ts=8:sts=8:sw=8:noexpandtab
