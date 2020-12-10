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
    r += strlen_P(r);
    return r;
}

static char* adddisp(char* out, int8_t v)
{
    *out++ = '$';
    if (v + 2 >= 0)
        *out++ = '+';
#if TEST
    out += sprintf(out, "%d", v + 2);
#else
    out = itoa(v + 2, out, 10);
#endif
    *out++ = '\0';
    return out - 1;
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
    *out++ = '\0';
    return out - 1;
}

char* addrp(char* out, uint8_t v)
{
    switch (v) {
        case 0: *out++ = 'b'; *out++ = 'c'; break;
        case 1: *out++ = 'd'; *out++ = 'e'; break;
        case 2: *out++ = 'h'; *out++ = 'l'; break;
        case 3: *out++ = 's'; *out++ = 'p'; break;
    }
    *out++ = '\0';
    return out - 1;
}

char* addcomma(char* out)
{
    *out++ = ',';
    *out++ = ' ';
    *out++ = '\0';
    return out - 1;
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
    return out - 1;
}

int disassemble(uint8_t mem[MAX_INST_SZ], char out[MAX_DISASM_SZ])
{
#define ADD(v) { nxt = add(nxt, PSTR(v)); *nxt++ = ' '; *nxt = '\0'; }
#define ADDR(v, n) { add(nxt, PSTR(v)); return n; }

    char* nxt = out;
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
                        ADD("add hl,"); addrp(nxt, p); return 1;
                    }
                    break;
                case 2:
                    if (q == 0) {
                        switch (p) {
                            case 0: ADDR("ld (bc), a", 1);
                            case 1: ADDR("ld (de), a", 1);
                            case 2: nxt = add(nxt, PSTR("ld (")); nxt = addnn(nxt, m1, m2); ADD("), hl"); return 3;
                            case 3: nxt = add(nxt, PSTR("ld (")); nxt = addnn(nxt, m1, m2); ADD("), a"); return 3;
                        }
                    } else {
                        switch (p) {
                            case 0: ADDR("ld a, (bc)", 1);
                            case 1: ADDR("ld a, (de)", 1);
                            case 2: nxt = add(nxt, PSTR("ld hl, (")); nxt = addnn(nxt, m1, m2); ADD(")"); return 3;
                            case 3: nxt = add(nxt, PSTR("ld a, (")); nxt = addnn(nxt, m1, m2); ADD(")"); return 3;
                        }
                    }
                    break;
                case 3:
                    if (q == 0) {
                        ADD("inc"); addrp(nxt, p); return 1;
                    } else {
                        ADD("dec"); addrp(nxt, p); return 1;
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
