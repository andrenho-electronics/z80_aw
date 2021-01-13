OBJ      = z80pres.o
TEST_OBJ = test/tests.o
A        = libz80pres.a
LIB      = ../libz80aw/libz80aw.a
CPPFLAGS = -Wall -Wextra -g -O0
CXXFLAGS = -std=c++17

all: $(A)

$(A): $(OBJ)
	$(AR) rcsT $@ $^ $(LIB)

tests: $(TEST_OBJ) $(A)
	$(CXX) $^ -o $@

check: tests
	./tests

check-leaks: tests
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./tests

.PHONY: clean
clean:
	rm -f *.o **/*.o $(A) tests

# vim: set ts=8 sts=8 sw=8 noexpandtab:
