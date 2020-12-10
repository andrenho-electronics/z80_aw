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
}

// vim:ts=4:sts=4:sw=4:expandtab
