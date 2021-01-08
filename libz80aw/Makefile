OBJ      = z80aw.o comm.o compiler.o contrib/map.o
TEST_OBJ = test/test.o
DLL      = libz80aw.so
TOML_LIB = ../contrib/tomlc99/libtoml.a
CPPFLAGS = -fPIC -Wall -Wextra -g -O0 -I../common

all: $(DLL)

$(DLL): $(OBJ)
	$(CC) $^ $(TOML_LIB) -shared -o $@

tests: $(TEST_OBJ) $(DLL)
	$(CC) $^ -o $@

check: tests
	LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:. ./tests -z

check-leaks: tests
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./tests

.PHONY: clean
clean:
	rm -f *.o **/*.o $(DLL) tests

# vim: set ts=8 sts=8 sw=8 noexpandtab:
