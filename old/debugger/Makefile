OBJ=main.o ui/ui.o ui/status.o ui/window.o ui/window.hh z80/Z80.o z80/z80_user.o config/config.o hardware/emulatedhardware.o hardware/realhardware.o hardware/hardware.o compiler/compiler.o ui/memory.o ui/source.o ui/terminal.o

CPPFLAGS=-Iz80 -DLSB_FIRST -Wall -Wextra -O0 -g
CXXFLAGS=-std=c++17
LDFLAGS=-Llib

ifeq ($(OS),Windows_NT)
	CPPFLAGS+=-Iinclude -Iinclude/pdcurses
	LDFLAGS+=-lpdcurses
else
	LDFLAGS+=-lncurses
endif

all: debugger

debugger: ${OBJ}
	${CXX} -o $@ $^ ${LDFLAGS}

.PHONY: clean
clean:
	rm -f debugger *.o **/*.o

# vim:ts=8:sts=8:sw=8:noexpandtab
