EXE=z80debugger
OBJECTS=main.o
CFLAGS=-Wall -Wextra -O2 -ggdb3 -I../commlib
LDFLAGS=-lreadline

.PHONY: all
all: ${EXE}

.PHONY: clean
clean:
	${RM} *.o ${EXE}

${EXE}: ${OBJECTS}
	${CC} ${CFLAGS} $^ ../commlib/commlib.a -o $@ ${LDFLAGS}

# vim: set ts=8 sts=8 sw=8 noexpandtab:
