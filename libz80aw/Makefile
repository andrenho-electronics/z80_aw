OBJ_COMM      = comm/z80aw.o comm/comm.o comm/compiler.o comm/logging.o contrib/map.o
OBJ_PRES      = pres/z80aw.o pres/z80pres.o pres/codeview.o pres/memoryview.o pres/terminalview.o pres/diskview.o
TEST_COMM_EXE = tests-comm
TEST_PRES_EXE = tests-pres
TEST_NMI_EXE  = tests-nmi
TEST_DISK_EXE = tests-disk
EMULATOR_EXE  = emulator
LIB           = libz80aw.a
TOML_LIB      = ../contrib/tomlc99/libtoml.a
CPPFLAGS      = -Wall -Wextra -g -O0 -I../common -DTEST_CPU
CFLAGS        = -std=c11 -D_DEFAULT_SOURCE
CXXFLAGS      = -std=c++17
CC            = gcc
CXX           = g++

all: $(LIB) $(EMULATOR_EXE)

tools: dump_mem_page
	
dump_mem_page: tools/dump_mem_page.o $(LIB)
	$(CC) $^ -o $@

dump_sdcard: tools/dump_sdcard.o $(LIB)
	$(CC) $^ -o $@

$(EMULATOR_EXE): emu/emulator.c emu/z80/Z80.c
	$(CC) $^ -o $@ $(CFLAGS) $(CPPFLAGS) -DLSB_FIRST -lutil

$(LIB): $(OBJ_COMM) $(OBJ_PRES) $(TOML_LIB)
	$(AR) rcsT $@ $^

$(TEST_COMM_EXE): tests/test-comm.o $(OBJ_COMM) $(TOML_LIB)
	$(CC) $^ -o $@

$(TEST_NMI_EXE): tests/test-nmi.o $(OBJ_COMM) $(TOML_LIB)
	$(CC) $^ -o $@

$(TEST_PRES_EXE): tests/test-pres.o $(OBJ_COMM) $(OBJ_PRES) $(TOML_LIB)
	$(CXX) $^ -o $@

$(TEST_DISK_EXE): tests/test-disk.o $(OBJ_COMM) $(TOML_LIB)
	$(CC) $^ -o $@

check: $(TEST_COMM_EXE) $(TEST_PRES_EXE) $(EMULATOR_EXE)
	./$(TEST_COMM_EXE) && ./$(TEST_PRES_EXE)

check-nmi: $(TEST_NMI_EXE) $(EMULATOR_EXE)
	./$<

check-nmi-real: $(TEST_NMI_EXE)
	./$< /dev/ttyUSB0

check-comm: $(TEST_COMM_EXE) $(EMULATOR_EXE)
	./$< -l

check-comm-real: $(TEST_COMM_EXE)
	./$< -l -r /dev/ttyUSB0

check-disk: $(TEST_DISK_EXE) $(EMULATOR_EXE)
	./$< -l

check-disk-real: $(TEST_DISK_EXE)
	./$< -l -r /dev/ttyUSB0

check-pres: $(TEST_PRES_EXE) $(EMULATOR_EXE)
	./$<

check-comm-leaks: $(TEST_COMM_EXE) $(EMULATOR_EXE)
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$<

check-pres-leaks: $(TEST_PRES_EXE) $(EMULATOR_EXE)
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$<

.PHONY: clean
clean:
	rm -f *.o **/*.o $(LIB) $(TEST_COMM_EXE) $(TEST_PRES_EXE) $(EMULATOR_EXE) dump_mem_page

# vim: set ts=8 sts=8 sw=8 noexpandtab:
