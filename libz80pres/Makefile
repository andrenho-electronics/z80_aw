OBJ      = code.o
TEST_OBJ = test.o
A        = libz80pres.a
TOML_LIB = ../contrib/tomlc99/libtoml.a
CPPFLAGS = -fPIC -Wall -Wextra -g -O0 -I../common -DTEST_CPU

all: $(A)

$(A): $(OBJ)
	$(AR) rcs $@ $^

tests: $(TEST_OBJ) $(A)
	$(CC) $^ -o $@ $(TOML_LIB)

check: tests
	LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:. ./tests -z -l

check-leaks: tests
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./tests

.PHONY: clean
clean:
	rm -f *.o **/*.o $(A) tests

# vim: set ts=8 sts=8 sw=8 noexpandtab:
