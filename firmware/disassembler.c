#include "disassembler.h"

#include <stdlib.h>
#include <string.h>

#if !TEST
#  include <avr/pgmspace.h>
#else
#  include <stdio.h>
#  define PSTR(v) v
#  define PGM_P const char* 
#  define pgm_read_byte *
#endif

static char* add(char* out, PGM_P s)
{
    char c;
    while ((c = pgm_read_byte(s++)) != 0)
        *out++ = c;
    *out = '\0';
    return out;
}

static char* add_number(int value, char* out, int base)
{
#if TEST
    out += sprintf(out, base == 10 ? "%d" : "%x", value);
    *out++ = '\0';
    return out - 1;
#else
    itoa(value, out, base);
    out += strlen(out);
    return out;
#endif
}

static char* adddisp(char* out, int8_t v)
{
    *out++ = '$';
    if (v + 2 >= 0)
        *out++ = '+';
    return add_number(v + 2, out, 10);
}

static char* addcc(char* out, uint8_t v)
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

static char* addrp(char* out, uint8_t v)
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

static char* addrp2(char* out, uint8_t v)
{
    if (v != 3) {
        return addrp(out, v);
    } else {
        *out++ = 'a'; *out++ = 'f'; *out = '\0';
        return out;
    }
}

static char* addcomma(char* out)
{
    *out++ = ',';
    *out++ = ' ';
    *out++ = '\0';
    return out - 1;
}

static char* addnn(char* out, uint8_t p1, uint8_t p2)
{
    uint16_t v = p1 | ((uint16_t) p2 << 8);
    out = add_number(v, out, 16);
    *out++ = 'h';
    *out++ = '\0';
    return out - 1;
}

static char* add_n(char* out, uint8_t v)
{
    out = add_number(v, out, 16);
    *out++ = 'h';
    *out++ = '\0';
    return out - 1;
}

static char* add_dec(char* out, uint8_t v)
{
    out = add_number(v, out, 10);
    return out;
}

static char* add_r(char* out, uint8_t p)
{
    switch (p) {
        case 0: *out++ = 'b'; break;
        case 1: *out++ = 'c'; break;
        case 2: *out++ = 'd'; break;
        case 3: *out++ = 'e'; break;
        case 4: *out++ = 'h'; break;
        case 5: *out++ = 'l'; break;
        case 6: out = add(out, PSTR("(hl)")); return out;
        case 7: *out++ = 'a'; break;
    }
    *out++ = '\0';
    return out - 1;
}

static char* add_alu(char* out, uint8_t p)
{
    switch (p) {
        case 0: out = add(out, PSTR("add a, ")); break;
        case 1: out = add(out, PSTR("adc a, ")); break;
        case 2: out = add(out, PSTR("sub ")); break;
        case 3: out = add(out, PSTR("sbc a, ")); break;
        case 4: out = add(out, PSTR("and ")); break;
        case 5: out = add(out, PSTR("xor ")); break;
        case 6: out = add(out, PSTR("or ")); break;
        case 7: out = add(out, PSTR("cp ")); break;
    }
    return out;
}

static char* addrot(char* out, uint8_t p)
{
    switch (p) {
        case 0: out = add(out, PSTR("rlc ")); break;
        case 1: out = add(out, PSTR("rrc ")); break;
        case 2: out = add(out, PSTR("rl ")); break;
        case 3: out = add(out, PSTR("rr ")); break;
        case 4: out = add(out, PSTR("sla ")); break;
        case 5: out = add(out, PSTR("sra ")); break;
        case 6: out = add(out, PSTR("sll ")); break;
        case 7: out = add(out, PSTR("srl ")); break;
    }
    return out;
}

static char* add_im(char* out, uint8_t p)
{
    switch (p) {
        case 0: case 4: *out++ = '0'; break;
        case 1: case 5: *out++ = '0'; *out++ = '/'; *out++ = '1'; break;
        case 2: case 6: *out++ = '1'; break;
        case 3: case 7: *out++ = '2'; break;
    }
    *out = '\0';
    return out;
}

static char* add_bli(char* nxt, uint8_t a, uint8_t b)
{
    switch (a) {
        case 4:
            switch (b) {
                case 0: return add(nxt, PSTR("ldi"));
                case 1: return add(nxt, PSTR("ldd"));
                case 2: return add(nxt, PSTR("ldir"));
                case 3: return add(nxt, PSTR("lddr"));
            }
            break;
        case 5:
            switch (b) {
                case 0: return add(nxt, PSTR("cpi"));
                case 1: return add(nxt, PSTR("cpd"));
                case 2: return add(nxt, PSTR("cpir"));
                case 3: return add(nxt, PSTR("cpdr"));
            }
            break;
        case 6:
            switch (b) {
                case 0: return add(nxt, PSTR("ini"));
                case 1: return add(nxt, PSTR("ind"));
                case 2: return add(nxt, PSTR("inir"));
                case 3: return add(nxt, PSTR("indr"));
            }
            break;
        case 7:
            switch (b) {
                case 0: return add(nxt, PSTR("outi"));
                case 1: return add(nxt, PSTR("outd"));
                case 2: return add(nxt, PSTR("otir"));
                case 3: return add(nxt, PSTR("otdr"));
            }
            break;
    }
    return nxt;
}

#define ADD(v) { nxt = add(nxt, PSTR(v)); *nxt++ = ' '; *nxt = '\0'; }
#define ADDR(v, n) { add(nxt, PSTR(v)); return n; }

static int cb_prefix(uint8_t* mem, char* nxt)
{
    uint8_t m = mem[0];
    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111;
    switch (x) {
        case 0:
            nxt = addrot(nxt, y); add_r(nxt, z);
            return 1;
        case 1:
            ADD("bit"); nxt = add_dec(nxt, y); nxt = addcomma(nxt); add_r(nxt, z);
            return 1;
        case 2:
            ADD("res"); nxt = add_dec(nxt, y); nxt = addcomma(nxt); add_r(nxt, z);
            return 1;
        case 3:
            ADD("set"); nxt = add_dec(nxt, y); nxt = addcomma(nxt); add_r(nxt, z);
            return 1;
    }
    return 1;
}

static int dd_prefix(uint8_t* mem, char* nxt)
{
    (void) mem; (void) nxt;
    return 0;  // TODO
}

static int ed_prefix(uint8_t* mem, char* nxt)
{
    uint8_t m = mem[0],
            m1 = mem[1],
            m2 = mem[2];
    
    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111,
            q = y & 1,
            p = y >> 1;

    switch (x) {
        //
        // x == 1
        //
        case 1:
            switch (z) {
                case 0:
                    if (y != 6) {
                        ADD("in"); nxt = add_r(nxt, y); ADD(", (c)"); return 1;
                    } else {
                        ADD("in (c)");
                    }
                    break;
                case 1:
                    if (y != 6) {
                        ADD("out"); nxt = add_r(nxt, y); ADD(", (c)"); return 1;
                    } else {
                        ADD("out (c), 0");
                    }
                    break;
                case 2:
                    if (q == 0) {
                        ADD("sbc hl, "); addrp(nxt, p); return 1;
                    } else {
                        ADD("adc hl, "); addrp(nxt, p); return 1;
                    }
                    break;
                case 3:
                    if (q == 0) {
                        nxt = add(nxt, PSTR("ld (")); nxt = addnn(nxt, m1, m2); nxt = add(nxt, PSTR(")")); nxt = addcomma(nxt); addrp(nxt, p); return 3;
                    } else {
                        nxt = add(nxt, PSTR("ld (")); nxt = addrp(nxt, p); ADD("),"); addnn(nxt, m1, m2); return 3;
                    }
                case 4:
                    ADD("neg"); return 1;
                case 5:
                    if (y != 1) ADD("retn") else ADD("reti"); return 1;
                case 6:
                    ADD("im"); add_im(nxt, y); return 1;
                case 7:
                    switch (y) {
                        case 0: ADD("ld i, a"); return 1;
                        case 1: ADD("ld r, a"); return 1;
                        case 2: ADD("ld a, i"); return 1;
                        case 3: ADD("ld a, r"); return 1;
                        case 4: ADD("rrd"); return 1;
                        case 5: ADD("rld"); return 1;
                        case 6: ADD("nop*"); return 1;
                        case 7: ADD("nop*"); return 1;
                    }
                    break;
            }
            break;

        //
        // x == 2
        // 
        case 2:
            if (z <= 3 && y >= 4) {
                add_bli(nxt, y, z);
                return 1;
            }
            break;
    }

    return 1;
}

static int fd_prefix(uint8_t* mem, char* nxt)
{
    (void) mem; (void) nxt;
    return 0;  // TODO
}

int disassemble(uint8_t* mem, char* nxt)
{
    uint8_t m = mem[0],
            m1 = mem[1],
            m2 = mem[2];
    
    // decoding based on http://www.z80.info/decoding.htm
    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111,
            q = y & 1,
            p = y >> 1;

    // printf("%d %d %d\n", x, y, z);

    switch (x) {
        //
        // x == 0
        //
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
                case 4:
                    ADD("inc"); add_r(nxt, y); return 1;
                case 5:
                    ADD("dec"); add_r(nxt, y); return 1;
                case 6:
                    ADD("ld"); nxt = add_r(nxt, y); nxt = addcomma(nxt); add_n(nxt, m1); return 2;
                case 7:
                    switch (y) {
                        case 0: ADDR("rlca", 1);
                        case 1: ADDR("rrca", 1);
                        case 2: ADDR("rla", 1);
                        case 3: ADDR("rra", 1);
                        case 4: ADDR("daa", 1);
                        case 5: ADDR("cpl", 1);
                        case 6: ADDR("scf", 1);
                        case 7: ADDR("ccf", 1);
                    }
                    break;
            }
            break;

        //
        // x == 1
        //
        case 1:
            if (z == 6 && y == 6) {
                ADDR("halt", 1);
            } else {
                ADD("ld"); nxt = add_r(nxt, y); nxt = addcomma(nxt); add_r(nxt, z);
                return 1;
            }
            break;

        //
        // x == 2
        //
        case 2:
            nxt = add_alu(nxt, y); add_r(nxt, z);
            return 1;

        //
        // x == 3
        //
        case 3:
            switch (z) {
                case 0:
                    ADD("ret"); addcc(nxt, y); return 1;
                case 1:
                    if (q == 0) {
                        ADD("pop"); addrp2(nxt, p); return 1;
                    } else {
                        switch (p) {
                            case 0: ADDR("ret", 1);
                            case 1: ADDR("exx", 1);
                            case 2: ADDR("jp hl", 1);
                            case 3: ADDR("ld sp, hl", 1);
                        }
                    }
                    break;
                case 2:
                    ADD("jp"); nxt = addcc(nxt, y); nxt = addcomma(nxt); addnn(nxt, m1, m2);
                    return 3;
                case 3:
                    switch (y) {
                        case 0: ADD("jp"); addnn(nxt, m1, m2); return 3;
                        case 1: return cb_prefix(&mem[1], nxt) + 1;
                        case 2: nxt = add(nxt, PSTR("out (")); nxt = add_n(nxt, m1); ADD("), a"); return 2;
                        case 3: nxt = add(nxt, PSTR("in a, (")); nxt = add_n(nxt, m1); ADD(")"); return 2;
                        case 4: ADDR("ex (sp), hl", 1);
                        case 5: ADDR("ex de, hl", 1);
                        case 6: ADDR("di", 1);
                        case 7: ADDR("ei", 1);
                    }
                    break;
                case 4:
                    ADD("call"); nxt = addcc(nxt, y); nxt = addcomma(nxt); addnn(nxt, m1, m2);
                    return 3;
                case 5:
                    if (q == 0) {
                        ADD("push"); nxt = addrp2(nxt, p); return 1;
                    } else {
                        switch (p) {
                            case 0: ADD("call"); addnn(nxt, m1, m2); return 3;
                            case 1: return dd_prefix(&mem[1], nxt) + 1;
                            case 2: return ed_prefix(&mem[1], nxt) + 1;
                            case 3: return fd_prefix(&mem[1], nxt) + 1;
                        }
                    }
                    break;
                case 6:
                    nxt = add_alu(nxt, y); add_n(nxt, m1); return 2;
                case 7:
                    ADD("rst"); add_n(nxt, y * 8); return 2;
            }
            break;
    }

    add(nxt, PSTR("UNKNOWN"));
    return 1;
}

#undef ADD

// vim:ts=4:sts=4:sw=4:expandtab
