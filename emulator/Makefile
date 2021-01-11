OBJ = main.o z80/Z80.o# z80/Debug.o
EXE = emulator
CPPFLAGS = -Wall -Wextra -g -O0 -I../common -DLSB_FIRST# -DDEBUG
LDFLAGS = -lutil

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $^ -o $@ -lutil

.PHONY: clean
clean:
	rm -f *.o **/*.o $(DLL) tests emulator

# vim: set ts=8 sts=8 sw=8 noexpandtab:
