OBJECTS=commlib.o
CFLAGS=-Wall -Wextra -O2 -ggdb3

.PHONY: all
all: commlib.a

.PHONY: clean
clean:
	${RM} *.o commlib.a

commlib.a: ${OBJECTS}
	${AR} rcs $@ $^

# vim: set ts=8 sts=8 sw=8 noexpandtab:
