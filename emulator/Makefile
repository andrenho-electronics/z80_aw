OBJ = main.o
EXE = emulator
CPPFLAGS = -Wall -Wextra -g -O0 -I../common
LDFLAGS = -lutil

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $^ -o $@ -lutil

.PHONY: clean
clean:
	rm -f *.o $(DLL) tests

# vim: set ts=8 sts=8 sw=8 noexpandtab:
