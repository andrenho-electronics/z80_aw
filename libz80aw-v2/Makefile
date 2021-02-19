OBJECTS       = serial.o z80aw.o
LIB           = libz80aw.a
CPPFLAGS      = -Wall -Wextra -g -O0 -I../common -DTEST_CPU
CFLAGS        = -std=c11 -D_DEFAULT_SOURCE
CXXFLAGS      = -std=c++17
CC            = gcc
CXX           = g++

all: $(LIB)

$(LIB): $(OBJECTS)
	$(AR) rcsT $@ $^

test-z80aw: tests/test-z80aw.o
	$(CXX) $^ -o $@

check-z80aw-real: test-z80aw
	./$< -r /dev/ttyUSB0

.PHONY: clean
	$(RM) -f *.o **/*.o $(LIB) test-z80aw

# vim: set ts=8 sts=8 sw=8 noexpandtab:
