#include "disassembler.h"

// TODO: DDCB/FDCB prefixed opcodes

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
#define ZP(s, ...) return z80_print(buf, PSTR(s), ##__VA_ARGS__)

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

static char* z80_print_alu(char* buf, int alu)
{
    switch (alu) {
        case 0: RADD("add a,");
        case 1: RADD("adc a,");
        case 2: RADD("sub");
        case 3: RADD("sbc a,");
        case 4: RADD("and");
        case 5: RADD("xor");
        case 6: RADD("or");
        case 7: RADD("cp");
    }
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

static char* z80_print_rotation(char* buf, uint8_t v)
{
    switch (v) {
        case 0: RADD("rlc");
        case 1: RADD("rrc");
        case 2: RADD("rl");
        case 3: RADD("rr");
        case 4: RADD("sla");
        case 5: RADD("sra");
        case 6: RADD("sll");
        case 7: RADD("srl");
    }
    return buf;
}

static char* z80_print_immediate(char* buf, uint8_t v)
{
    switch (v) {
        case 0: case 4: RADD("0");
        case 1: case 5: RADD("0/1");
        case 2: case 6: RADD("1");
        case 3: case 7: RADD("2");
    }
    return buf;
}

static char* z80_print_block(char* buf, uint8_t a, uint8_t b)
{
    switch (b) {
        case 0:
            switch (a) {
                case 4: RADD("ldi");
                case 5: RADD("ldd");
                case 6: RADD("ldir");
                case 7: RADD("lddr");
            }
            break;
        case 1:
            switch (a) {
                case 4: RADD("cpi");
                case 5: RADD("cpd");
                case 6: RADD("cpir");
                case 7: RADD("cpdr");
            }
            break;
        case 2:
            switch (a) {
                case 4: RADD("ini");
                case 5: RADD("ind");
                case 6: RADD("inir");
                case 7: RADD("indr");
            }
            break;
        case 3:
            switch (a) {
                case 4: RADD("outi");
                case 5: RADD("outd");
                case 6: RADD("otir");
                case 7: RADD("otdr");
            }
            break;
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

static char* z80_print_hex8(char* buf, uint8_t v)
{
    buf = z80_print_number(buf, v, 16);
    *buf++ = 'h';
    *buf = '\0';
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
    size_t mem_idx = 1;

    char c;
    while ((c = pgm_read_byte(fmt++)) != 0) {
        if (c == '%') {
            char o = pgm_read_byte(fmt++);
            switch (o) {
                case 'a':   // alu (parameter: command)
                    buf = z80_print_alu(buf, va_arg(ap, int));
                    break;
                case 'b': {   // block
                        int a = va_arg(ap, int);
                        int b = va_arg(ap, int);
                        buf = z80_print_block(buf, a, b);
                    }
                    break;
                case 'c':   // condition (parameter: condition)
                    buf = z80_print_condition(buf, va_arg(ap, int));
                    break;
                case 'd':   // displacement (parameters: memory)
                    buf = z80_print_displacement(buf, va_arg(ap, char*)[mem_idx], 2);
                    ++n_bytes;
                    break;
                case 'h':   // HL (may replace per IX/IY) - (parameters: prefix)
                    buf = z80_print_regpairs(buf, 2, va_arg(ap, int), 1);
                    break;
                case 'i':    // integer
                    buf = z80_print_number(buf, va_arg(ap, int), 10);
                    *buf = '\0';
                    break;
                case 'm':  // immediate
                    buf = z80_print_immediate(buf, va_arg(ap, int));
                    break;
                case 'n':     // hex8
                    buf = z80_print_hex8(buf, va_arg(ap, char*)[mem_idx]);
                    ++n_bytes;
                    break;
                case 'N': {   // hex16 (parameters: bytes 1 and 2)
                        uint8_t* mem = (uint8_t *) va_arg(ap, char*);
                        buf = z80_print_hex16(buf, mem[mem_idx], mem[mem_idx+1]);
                        n_bytes += 2;
                    }
                    break;
                case 'p': case 'P': {  // register pair (1 or 2) - may replace HL per IX/IY (parameters: register, prefix)
                        int p = va_arg(ap, int);
                        Z80Prefix prefix = va_arg(ap, int);
                        buf = z80_print_regpairs(buf, p, prefix, o == 'p' ? 1 : 2);
                    }
                    break;
                case 'r': {   // register (may replace HL per IX/IY) - parameters (register, prefix, memory buffer)
                         int p = va_arg(ap, int);
                         Z80Prefix prefix = va_arg(ap, int);
                         uint8_t* mem = (uint8_t *) va_arg(ap, char*);
                         buf = z80_print_register(buf, p, prefix, mem[mem_idx]);
                         if (p == 6 && prefix != NO_PREFIX) {  // DD/DF instruction type, (HL) is transformed in (IX/Y+d)
                             ++n_bytes;
                             ++mem_idx;
                         }
                     }
                     break;
                case 't':
                    buf = z80_print_rotation(buf, va_arg(ap, int));
                    break;
                case 'x':    // hex
                    buf = z80_print_number(buf, va_arg(ap, int), 16);
                    *buf++ = 'h';
                    *buf = '\0';
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

static int cb_prefix(uint8_t* mem, char* buf, Z80Prefix prefix)
{
    uint8_t m = mem[0];
    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111;

    switch (x) {
        case 0: ZP("%t %r", y, z, prefix, mem);
        case 1: ZP("bit %i, %r", y, z, prefix, mem);
        case 2: ZP("res %i, %r", y, z, prefix, mem);
        case 3: ZP("set %i, %r", y, z, prefix, mem);
    }

    return 1;
}

static int ed_prefix(uint8_t* mem, char* buf, Z80Prefix prefix)
{
    uint8_t m = mem[0];
    uint8_t x = m >> 6,
            y = (m >> 3) & 0b111,
            z = m & 0b111,
            q = y & 1,
            p = y >> 1;

    if (x == 1) {
        switch (z) {
            case 0:
                if (y != 6)
                    ZP("in %r, (c)", y, prefix, mem);
                else
                    ZP("in (c)");
            case 1:
                if (y != 6)
                    ZP("out (c), %r", y, prefix, mem);
                else
                    ZP("out (c), 0");
            case 2:
                if (q == 0)
                    ZP("sbc %h, %p", prefix, p);
                else
                    ZP("adc %h, %p", prefix, p);
            case 3:
                if (q == 0)
                    ZP("ld (%N), %p", mem, p);
                else
                    ZP("ld %p, (%N)", p, mem);
            case 4:
                ZP("neg");
            case 5:
                if (y != 1)
                    ZP("retn");
                else
                    ZP("reti");
            case 6:
                ZP("im %m", y);
            case 7:
                switch (y) {
                    case 0: ZP("ld i, a");
                    case 1: ZP("ld r, a");
                    case 2: ZP("ld a, i");
                    case 3: ZP("ld a, r");
                    case 4: ZP("rrd");
                    case 5: ZP("rld");
                    case 6: ZP("nop");
                    case 7: ZP("nop");
                }
                break;
        }
    } else if (x == 2) {
        if (z <= 3 && y >= 4)
            ZP("%b", y, z);
    }

    return 1;
}

int disassemble(uint8_t* mem, char* buf, Z80Prefix prefix)
{
    uint8_t m = mem[0];

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
                        case 2: ZP("djnz %d", mem);
                        case 3: ZP("jr %d", mem);
                        default: ZP("jr %c, %d", y-4, mem);
                    }
                    break;
                case 1:
                    if (q == 0)
                        ZP("ld %p, %N", p, prefix, mem);
                    else
                        ZP("add %h, %p", prefix, p, prefix);
                    break;
                case 2:
                    if (q == 0) {
                        switch (p) {
                            case 0: ZP("ld (bc), a");
                            case 1: ZP("ld (de), a");
                            case 2: ZP("ld (%N), %h", mem, prefix);
                            case 3: ZP("ld (%N), a", mem);
                        }
                    } else {
                        switch (p) {
                            case 0: ZP("ld a, (bc)");
                            case 1: ZP("ld a, (de)");
                            case 2: ZP("ld %h, (%N)", prefix, mem);
                            case 3: ZP("ld a, (%N)", mem);
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
                    ZP("inc %r", y, prefix, mem);
                    break;
                case 5:
                    ZP("dec %r", y, prefix, mem);
                    break;
                case 6:
                    ZP("ld %r, %n", y, prefix, mem, mem);
                case 7:
                    switch (y) {
                        case 0: ZP("rlca");
                        case 1: ZP("rrca");
                        case 2: ZP("rla");
                        case 3: ZP("rra");
                        case 4: ZP("daa");
                        case 5: ZP("cpl");
                        case 6: ZP("scf");
                        case 7: ZP("ccf");
                    }
                    break;
            }
            break;

        //
        // x == 1
        //
        case 1:
            if (y == 6 && prefix == NO_PREFIX) {
                ZP("halt");
            } else {
                ZP("ld %r, %r", y, prefix, mem, z, prefix, mem);
            }
            break;

        //
        // x == 2
        //
        case 2:
            ZP("%a %r", y, z, prefix, mem);

        //
        // x == 3
        //
        case 3:
            switch (z) {
                case 0:
                    ZP("ret %c", y);
                case 1:
                    if (q == 0) {
                        ZP("pop %P", p);
                    } else {
                        switch (p) {
                            case 0: ZP("ret");
                            case 1: ZP("exx");
                            case 2: ZP("jp (%h)", prefix);
                            case 3: ZP("ld sp, %h", prefix);
                        }
                    }
                    break;
                case 2:
                    ZP("jp %c, %N", y, mem);
                case 3:
                    switch (y) {
                        case 0: ZP("jp %N", mem);
                        case 1: return cb_prefix(&mem[1], buf, prefix) + 1;
                        case 2: ZP("out (%n), a", mem);
                        case 3: ZP("in a, (%n)", mem);
                        case 4: ZP("ex (sp), %h", prefix);
                        case 5: ZP("ex de, hl");
                        case 6: ZP("di");
                        case 7: ZP("ei");
                    }
                    break;
                case 4:
                    ZP("call %c, %N", y, mem);
                case 5:
                    if (q == 0) {
                        ZP("push %P", p);
                    } else {
                        switch (p) {
                            case 0: ZP("call %N", mem);
                            case 1: return disassemble(&mem[1], buf, DD) + 1;
                            case 2: return ed_prefix(&mem[1], buf, prefix) + 1;
                            case 3: return disassemble(&mem[1], buf, FD) + 1;
                        }
                    }
                    break;
                case 6:
                    ZP("%a %n", y, mem);
                case 7:
                    ZP("rst %x", y * 8);
            }
            break;
    }

    ZP("Unknown");
}

#undef RADD
#undef ZP

// vim:ts=4:sts=4:sw=4:expandtab
