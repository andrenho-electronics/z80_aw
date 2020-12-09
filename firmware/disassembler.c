#include "disassembler.h"

#if !TEST
#  include <avr/pgmspace.h>
#else
#  include <string.h>
#  define strcpy_P strcpy
#  define strlen_P strlen
#  define PSTR(v) v
#  define PGM_P const char* 
#endif

static char* add(char* out, PGM_P s)
{
    char* r = strcpy_P(out, s);
    r += strlen_P(r) + 1;
    r[-1] = ' ';
    r[0] = '\0';
    return r;
}

static char* addhex4(char* out, uint8_t data)
{
    *out = data < 0xa ? data + '0' : data - 0xa + 'A';
    return out + 1;
}

static char* addhex16(char* out, uint16_t v)
{
    out = addhex4(out, v >> 12);
    out = addhex4(out, (v >> 8) & 0xf);
    out = addhex4(out, (v >> 4) & 0xf);
    out = addhex4(out, v & 0xf);
    *out = 'h';
    *(out + 1) = '\0';
    return out + 2;
}

int disassemble(uint8_t mem[MAX_INST_SZ], char out[MAX_DISASM_SZ])
{
#define ADD(v) nxt = strcpy_P(out, PSTR(v " "));

    char* nxt;

    switch (mem[0]) {
        case 0x0:
            add(out, PSTR("nop"));
            return 1;
        case 0xc3:
            nxt = add(out, PSTR("jp"));
            addhex16(nxt, mem[1] | (mem[2] << 8));
            return 3;
        default:
            add(out, PSTR("UNKNOWN"));
            return 1;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
