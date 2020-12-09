#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

#include <stdint.h>

#define MAX_INST_SZ 8
#define MAX_DISASM_SZ 40

int disassemble(uint8_t mem[MAX_INST_SZ], char out[MAX_DISASM_SZ]);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
