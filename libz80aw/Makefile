OBJ_COMM      = comm/z80aw.o comm/comm.o comm/compiler.o contrib/map.o
OBJ_PRES      = pres/z80aw.o pres/z80pres.o pres/codeview.o
TEST_COMM_OBJ = tests/test-comm.o
TEST_COMM_EXE = tests-comm
TEST_PRES_OBJ = tests/test-pres.o
TEST_PRES_EXE = tests-pres
EMULATOR_EXE  = emulator
LIB           = libz80aw.a
TOML_LIB      = ../contrib/tomlc99/libtoml.a
CPPFLAGS      = -Wall -Wextra -g -O0 -I../common -DTEST_CPU
CFLAGS        = -std=c11 -D_DEFAULT_SOURCE
CXXFLAGS      = -std=c++17

all: $(LIB) $(EMULATOR_EXE)

$(EMULATOR_EXE): emu/emulator.c emu/z80/Z80.c
	$(CC) $^ -o $@ $(CFLAGS) $(CPPFLAGS) -DLSB_FIRST -lutil

$(LIB): $(OBJ_COMM) $(OBJ_PRES) $(TOML_LIB)
	$(AR) rcsT $@ $^

$(TEST_COMM_EXE): $(TEST_COMM_OBJ) $(OBJ_COMM) $(TOML_LIB)
	$(CC) $^ -o $@

$(TEST_PRES_EXE): $(TEST_PRES_OBJ) $(OBJ_COMM) $(OBJ_PRES) $(TOML_LIB)
	$(CXX) $^ -o $@

check: $(TEST_COMM_EXE) $(TEST_PRES_EXE) $(EMULATOR_EXE)
	./$(TEST_COMM_EXE) && ./$(TEST_PRES_EXE)

check-comm: $(TEST_COMM_EXE) $(EMULATOR_EXE)
	./$< -z -l

check-comm-real: $(TEST_COMM_EXE)
	./$< -z -l -r /dev/ttyUSB0

check-pres: $(TEST_PRES_EXE) $(EMULATOR_EXE)
	./$<

check-comm-leaks: $(TEST_COMM_EXE) $(EMULATOR_EXE)
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$<

check-pres-leaks: $(TEST_COMM_EXE) $(EMULATOR_EXE)
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$<

.PHONY: clean
clean:
	rm -f *.o **/*.o $(LIB) $(TEST_COMM_EXE) $(TEST_PRES_EXE) $(EMULATOR_EXE)

# vim: set ts=8 sts=8 sw=8 noexpandtab:
