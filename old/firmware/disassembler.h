#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

#include <stdint.h>

#define MAX_INST_SZ 8
#define MAX_DISASM_SZ 40

typedef enum { NO_PREFIX, DD, FD } Z80Prefix;

int disassemble(uint8_t* mem, char* out, Z80Prefix prefix);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
