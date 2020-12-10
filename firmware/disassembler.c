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

char* addcc(char* out, uint8_t v)
{
    switch (v) {
        case 0: *out++ = 'n'; *out++ = 'z'; break;
        case 1: *out++ = 'z'; break;
        case 2: *out++ = 'n'; *out++ = 'c'; break;
        case 3: *out++ = 'c'; break;
        case 4: *out++ = 'p'; *out++ = 'o'; break;
        case 5: *out++ = 'p'; *out++ = 'e'; break;
        case 6: *out++ = 'p'; break;
        case 7: *out++ = 'm'; break;
    }
    return out;
}

char* addrp(char* out, uint8_t v)
{
    switch (v) {
        case 0: *out++ = 'b'; *out++ = 'c'; break;
        case 1: *out++ = 'd'; *out++ = 'e'; break;
        case 2: *out++ = 'h'; *out++ = 'l'; break;
        case 3: *out++ = 's'; *out++ = 'p'; break;
    }
    return out;
}

char* addcomma(char* out)
{
    *out++ = ',';
    *out++ = ' ';
    return out;
}

char* addnn(char* out, uint8_t p1, uint8_t p2)
{
    uint16_t v = p1 | ((uint16_t) p2 << 8);
#if TEST
    out = out + sprintf(out, "%x", v);
#else
    out = itoa(v, out, 16);
#endif
    *out++ = 'h';
    *out++ = '\0';
    return out;
}

int disassemble(uint8_t mem[MAX_INST_SZ], char out[MAX_DISASM_SZ])
{
#define ADD(v) nxt = add(out, PSTR(v))
#define ADDR(v, n) { add(out, PSTR(v)); return n; }

    char* nxt;
    uint8_t m = mem[0],
            m1 = mem[1],
            m2 = mem[2];
    
    // special tables
    if (m == 0xcb || m == 0xdd || m == 0xed || m == 0xfd) {
        goto not_found;  // TODO
    }

    // decoding based on http://www.z80.info/decoding.htm
    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111,
            q = y & 1,
            p = y >> 1;

    // printf("%d %d %d\n", x, y, z);

    switch (x) {
        case 0:
            switch (z) {
                case 0:
                    switch (y) {
                        case 0: ADDR("nop", 1);
                        case 1: ADDR("ex af, af'", 1);
                        case 2: ADD("djnz"); adddisp(nxt, m1); return 2;
                        case 3: ADD("jr"); adddisp(nxt, m1); return 2;
                        default: ADD("jr"); nxt = addcc(nxt, y-4); nxt = addcomma(nxt); adddisp(nxt, m1); return 2;
                    }
                    break;
                case 1:
                    if (q == 0) {
                        ADD("ld"); nxt = addrp(nxt, p); nxt = addcomma(nxt); addnn(nxt, m1, m2); return 3;
                    } else {
                        ADD("add hl, "); addrp(nxtm p); return 1;
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
