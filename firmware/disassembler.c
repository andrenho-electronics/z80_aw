#include "disassembler.h"

#include <stdlib.h>

#if !TEST
#  include <avr/pgmspace.h>
#else
#  include <string.h>
#  include <stdio.h>
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

static char* adddisp(char* out, int8_t v)
{
    *out++ = '$';
    if (v + 2 >= 0)
        *out++ = '+';
#if TEST
    return out + sprintf(out, "%d", v + 2);
#else
    return itoa(v + 2, out, 10);
#endif
}

int disassemble(uint8_t mem[MAX_INST_SZ], char out[MAX_DISASM_SZ])
{
#define ADD(v) nxt = add(out, PSTR(v))
#define ADDR(v, n) { add(out, PSTR(v)); return n; }

    char* nxt;
    uint8_t m = mem[0];
    
    // special tables
    if (m == 0xcb || m == 0xdd || m == 0xed || m == 0xfd) {
        goto not_found;  // TODO
    }

    // decoding based on http://www.z80.info/decoding.htm
    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111;

    // printf("%d %d %d\n", x, y, z);

    switch (x) {
        case 0:
            switch (z) {
                case 0:
                    switch (y) {
                        case 0: ADDR("nop", 1);
                        case 1: ADDR("ex af, af'", 1);
                        case 2: ADD("djnz"); adddisp(nxt, mem[1]); return 2;
                        case 3: ADD("jr"); adddisp(nxt, mem[1]); return 2;
                        default: ADD("jr"); nxt = addcc(y-4); nxt = addcomma(nxt); adddisp(nxt, mem[1]); return 2;
                    }
                    break;
            }
            break;
    }

not_found:
    add(out, PSTR("UNKNOWN"));
    return 1;

#undef ADD
}

// vim:ts=4:sts=4:sw=4:expandtab
