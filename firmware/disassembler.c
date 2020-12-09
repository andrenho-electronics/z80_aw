#include "disassembler.h"

#include <avr/pgmspace.h>

int disassemble(uint8_t mem[MAX_INST_SZ], char out[MAX_DISASM_SZ])
{
    strcpy_P(out, PSTR("Unknown"));
    return 1;
}

// vim:ts=4:sts=4:sw=4:expandtab
