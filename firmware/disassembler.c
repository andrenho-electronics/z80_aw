#include "disassembler.h"

/*

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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
#define ADDN(v) { nxt = add(nxt, PSTR(v)); }
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

static int dd_fd_prefix(uint8_t* mem, char* nxt, PGM_P reg)
{
#define ADDREG() { nxt = add(nxt, reg); }
    uint8_t m = mem[0],
            m1 = mem[1],
            m2 = mem[2];
    
    switch(m) {
        case 0x9:
            ADD("add"); ADDREG(); ADD(", bc"); return 1;
        case 0x19:
            ADD("add"); ADDREG(); ADD(", de"); return 1;
        case 0x21:
            ADD("ld"); ADDREG(); nxt = add(nxt, PSTR(", ")); addnn(nxt, m1, m2);
            return 3;
        case 0x22:
            ADDN("ld ("); nxt = addnn(nxt, m1, m2); ADD("),"); ADDREG();
            return 3;
        case 0x23:
            ADD("inc"); ADDREG();
            return 1;
    }

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
                            case 1: return dd_fd_prefix(&mem[1], nxt, PSTR("ix")) + 1;
                            case 2: return ed_prefix(&mem[1], nxt) + 1;
                            case 3: return dd_fd_prefix(&mem[1], nxt, PSTR("iy")) + 1;
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
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if !TEST
#  include <avr/pgmspace.h>
#else
#  include <stdio.h>
#  define PSTR(v) v
#  define PGM_P const char* 
#  define pgm_read_byte *
#endif

#define RADD(s) return z80_addstr(buf, PSTR(s))

static int z80_print(char* buf, PGM_P fmt, ...);

static char* z80_print_number(char* buf, int value, int base)
{
#if TEST
    buf += sprintf(buf, base == 10 ? "%d" : "%x", value);
    *buf++ = '\0';
    return buf - 1;
#else
    itoa(value, buf, base);
    buf += strlen(buf);
    return buf;
#endif
}

static char* z80_addstr(char* buf, PGM_P s)
{
    char c;
    while ((c = pgm_read_byte(s++)) != 0)
        *buf++ = c;
    *buf = '\0';
    return buf;
}

static char* z80_print_displacement(char* buf, int8_t v, int plus)
{
    *buf++ = '$';
    if (v + plus >= 0)
        *buf++ = '+';
    return z80_print_number(buf, v + plus, 10);
}

static char* z80_print_condition(char* buf, uint8_t v)
{
    switch (v) {
        case 0: RADD("nz");
        case 1: RADD("z");
        case 2: RADD("nc");
        case 3: RADD("c");
        case 4: RADD("po");
        case 5: RADD("pe");
        case 6: RADD("p");
        case 7: RADD("m");
    }
    return buf;
}

static char* z80_print_regpairs(char* buf, uint8_t v, Z80Prefix prefix, int type)
{
    switch (v) {
        case 0: RADD("bc");
        case 1: RADD("de");
        case 2:
            switch (prefix) {
                case NO_PREFIX: RADD("hl");
                case DD: RADD("ix");
                case FD: RADD("iy");
            }
            break;
        case 3:
            if (type == 1)
                RADD("sp");
            else
                RADD("af");
    }
    return buf;
}

static char* z80_print_register(char* buf, uint8_t v, Z80Prefix prefix, int8_t value)
{
    switch (v) {
        case 0: RADD("b");
        case 1: RADD("c");
        case 2: RADD("d");
        case 3: RADD("e");
        case 4: RADD("h");
        case 5: RADD("l");
        case 6:
            if (prefix == NO_PREFIX) {
                RADD("(hl)");
            } else {
                buf = z80_addstr(buf, prefix == DD ? PSTR("(ix") : PSTR("(iy"));
                if (value >= 0)
                    *buf++ = '+';
                buf = z80_print_number(buf, value, 10);
                return z80_addstr(buf, PSTR(")"));
            }
            break;
        case 7: RADD("a");
    }
    return buf;
}

static char* z80_print_hex16(char* buf, uint8_t p1, uint8_t p2)
{
    uint16_t v = p1 | ((uint16_t) p2 << 8);
    buf = z80_print_number(buf, v, 16);
    *buf++ = 'h';
    *buf = '\0';
    return buf;
}

static int z80_print(char* buf, PGM_P fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int n_bytes = 1;

    char c;
    while ((c = pgm_read_byte(fmt++)) != 0) {
        if (c == '%') {
            char o = pgm_read_byte(fmt++);
            switch (o) {
                case 'd':   // displacement
                    buf = z80_print_displacement(buf, va_arg(ap, int), 2);
                    ++n_bytes;
                    break;
                case 'c':   // condition
                    buf = z80_print_condition(buf, va_arg(ap, int));
                    break;
                case 'p': case 'P': {  // register pair (1 or 2) - may replace HL per IX/IY
                        int p = va_arg(ap, int);
                        Z80Prefix prefix = va_arg(ap, int);
                        buf = z80_print_regpairs(buf, p, prefix, o == 'p' ? 1 : 2);
                    }
                    break;
                case 'N': {   // hex16
                        int m1 = va_arg(ap, int);
                        int m2 = va_arg(ap, int);
                        buf = z80_print_hex16(buf, m1, m2);
                        n_bytes += 2;
                    }
                    break;
                case 'h':   // HL (may replace per IX/IY)
                    buf = z80_print_regpairs(buf, 2, va_arg(ap, int), 1);
                    break;
                case 'r': {   // register (may replace HL per IX/IY)
                         int p = va_arg(ap, int);
                         Z80Prefix prefix = va_arg(ap, int);
                         int m1 = va_arg(ap, int);
                         buf = z80_print_register(buf, p, prefix, m1);
                         if (p == 6 && prefix != NO_PREFIX)   // DD/DF instruction type, (HL) is transformed in (IX/Y+d)
                             ++n_bytes;
                     }
                     break;
#if TEST
                default:
                    fprintf(stderr, "Invalid option '%c'.\n", o);
                    exit(1);
#endif
            }
        } else {
            *buf++ = c;
        }
    }
    *buf = '\0';

    va_end(ap);
    return n_bytes;
}

int disassemble(uint8_t* mem, char* buf, Z80Prefix prefix)
{
#define ZP(s, ...) return z80_print(buf, PSTR(s), ##__VA_ARGS__)

    uint8_t m = mem[0],
            m1 = mem[1],
            m2 = mem[2];

    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111,
            q = y & 1,
            p = y >> 1;

    // printf("%d %d %d %d %d\n", x, y, z, q, p);

    switch (x) {
        //
        // x == 0
        //
        case 0:
            switch (z) {
                case 0:
                    switch (y) {
                        case 0: ZP("nop");
                        case 1: ZP("ex af, af'");
                        case 2: ZP("djnz %d", m1);
                        case 3: ZP("jr %d", m1);
                        default: ZP("jr %c, %d", y-4, m1);
                    }
                    break;
                case 1:
                    if (q == 0)
                        ZP("ld %p, %N", p, prefix, m1, m2);
                    else
                        ZP("add %h, %p", prefix, p, prefix);
                    break;
                case 2:
                    if (q == 0) {
                        switch (p) {
                            case 0: ZP("ld (bc), a");
                            case 1: ZP("ld (de), a");
                            case 2: ZP("ld (%N), %h", m1, m2, prefix);
                            case 3: ZP("ld (%N), a", m1, m2);
                        }
                    } else {
                        switch (p) {
                            case 0: ZP("ld a, (bc)");
                            case 1: ZP("ld a, (de)");
                            case 2: ZP("ld %h, (%N)", prefix, m1, m2);
                            case 3: ZP("ld a, (%N)", m1, m2);
                        }
                    }
                    break;
                case 3:
                    if (q == 0)
                        ZP("inc %p", p, prefix);
                    else
                        ZP("dec %p", p, prefix);
                    break;
                case 4:
                    ZP("inc %r", y, prefix, m1);
                    break;
                case 5:
                    ZP("dec %r", y, prefix, m1);
                    break;
            }
            break;

        // ...

        //
        // x == 3
        //
        case 3:
            switch (z) {
                case 5:
                    if (q == 0) {
                        // ...
                    } else {
                        switch (p) {
                            case 0: ZP("call %N", m1, m2);
                            case 1: return disassemble(&mem[1], buf, DD) + 1;
                            case 2: /* TODO */ break;
                            case 3: return disassemble(&mem[1], buf, FD) + 1;
                        }
                    }
                    break;
            }
            break;
    }

    ZP("Unknown");
#undef ZP
}

// vim:ts=4:sts=4:sw=4:expandtab
