#include "disassembler.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(s, ...) check_(s, (uint8_t[]){__VA_ARGS__})

void check_(const char* inst, uint8_t* args) {
    char buf[MAX_DISASM_SZ];
    disassemble(args, buf);
    while (buf[strlen(buf) - 1] == ' ')  // trim
        buf[strlen(buf) - 1] = '\0';
    if (strcmp(buf, inst) != 0) {
        printf("Error. Expected '%s', found '%s' (byte 0x%02x).\n", inst, buf, args[0]);
        exit(1);
    } else {
        printf("Ok: %s\n", buf);
    }
}

int main()
{
    CHECK("nop", 0x0);
    CHECK("ex af, af'", 0x8);
    CHECK("djnz $+8", 0x10, 0x6);
    CHECK("djnz $-107", 0x10, 0x93);
    CHECK("djnz $+0", 0x10, 0xfe);
    CHECK("djnz $+129", 0x10, 0x7f);
    CHECK("djnz $-126", 0x10, 0x80);
    CHECK("jr $+6", 0x18, 0x4);
    CHECK("jr nc, $+10", 0x30, 0x8);
    CHECK("ld bc, 302h", 0x01, 0x2, 0x3);
    CHECK("add hl, de", 0x19);
    CHECK("ld (1234h), hl", 0x22, 0x34, 0x12);
    CHECK("inc bc", 0x3);
    CHECK("inc (hl)", 0x34);
    CHECK("ld h, f0h", 0x26, 0xf0);
    CHECK("ld (hl), 12h", 0x36, 0x12);
    CHECK("cpl", 0x2f);
    CHECK("rrca", 0xf);
    CHECK("halt", 0x76);
    CHECK("ld h, (hl)", 0x66);
    CHECK("sbc a, e", 0x9b);
    CHECK("ret po", 0xe0);
    CHECK("pop af", 0xf1);
    CHECK("ld sp, hl", 0xf9);
    CHECK("jp pe, 1234h", 0xea, 0x34, 0x12);
    CHECK("jp c3c3h", 0xc3, 0xc3, 0xc3);
    CHECK("out (34h), a", 0xd3, 0x34);
    CHECK("push af", 0xf5);
    CHECK("xor 7bh", 0xee, 0x7b);
    CHECK("rst 20h", 0xe7);
}

// vim:ts=4:sts=4:sw=4:expandtab
