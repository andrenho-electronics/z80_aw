OBJ = z80aw.o comm.o
DLL = libz80aw.so
CPPFLAGS = -fPIC -Wall -Wextra -g -O0

all: $(DLL)

$(DLL): $(OBJ)
	$(CC) $^ -shared -o $@

tests: test.o $(DLL)
	$(CC) $^ -o $@

check: tests
	./tests

.PHONY: clean
clean:
	rm -f *.o $(DLL) tests

# vim: set ts=8 sts=8 sw=8 noexpandtab:
