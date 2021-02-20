OBJECTS       = serial.o z80aw.o realhardware.o proto/messages.pb.o
LIB           = libz80aw.a
CPPFLAGS      = -Wall -Wextra -g -O0 -I../common -I. -DTEST_CPU `pkg-config --cflags protobuf`
CFLAGS        = -std=c11 -D_DEFAULT_SOURCE
CXXFLAGS      = -std=c++17
LDFLAGS       = `pkg-config --libs protobuf`
CC            = gcc
CXX           = g++

all: $(LIB)

$(LIB): $(OBJECTS)
	$(AR) rcsT $@ $^

proto/messages.pb.o: proto/messages.pb.cc

proto/messages.pb.cc: ../common/messages.proto
	protoc -I=../common/ --cpp_out=proto ../common/messages.proto

test-z80aw: tests/test-z80aw.o $(LIB)
	$(CXX) $^ -o $@ $(LDFLAGS)

check-z80aw-real: test-z80aw
	./$< -r /dev/ttyUSB0

.PHONY: clean
clean:
	$(RM) -f *.o **/*.o $(LIB) test-z80aw proto/*.pb.*

# vim: set ts=8 sts=8 sw=8 noexpandtab:
