EXE=z80debugger
OBJECTS=main.o args.o commands.o
CFLAGS=-Wall -Wextra -O2 -ggdb3 -I../commlib -I../controller
LDFLAGS=-lreadline

.PHONY: all
all: ${EXE}

.PHONY: clean
clean:
	${RM} *.o ${EXE}

${EXE}: ${OBJECTS} ../commlib/commlib.a
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

# vim: set ts=8 sts=8 sw=8 noexpandtab:
