OBJ      = z80aw.o comm.o compiler.o contrib/map.o c++/z80aw.o
TEST_OBJ = test/test.o
A        = libz80aw.a
TOML_LIB = ../contrib/tomlc99/libtoml.a
CPPFLAGS = -Wall -Wextra -g -O0 -I../common -DTEST_CPU
CXXFLAGS = -std=c++17

all: $(A)

$(A): $(OBJ)
	$(AR) rcsT $@ $^ $(TOML_LIB)

tests: $(TEST_OBJ) $(A)
	$(CXX) $^ -o $@

check: tests
	./tests -z -l

check-real: tests
	./tests -z -l -r /dev/ttyUSB0

check-leaks: tests
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./tests

.PHONY: clean
clean:
	rm -f *.o **/*.o $(A) tests

# vim: set ts=8 sts=8 sw=8 noexpandtab:
