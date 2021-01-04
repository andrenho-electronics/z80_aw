OBJ      = z80aw.o comm.o
TEST_OBJ = test/test.o test/testutil.o
DLL      = libz80aw.so
CPPFLAGS = -fPIC -Wall -Wextra -g -O0 -I../common

all: $(DLL)

$(DLL): $(OBJ)
	$(CC) $^ -shared -o $@

tests: $(TEST_OBJ) $(DLL)
	$(CC) $^ -o $@

check: test
	./tests

.PHONY: clean
clean:
	rm -f *.o **/*.o $(DLL) tests

# vim: set ts=8 sts=8 sw=8 noexpandtab:
