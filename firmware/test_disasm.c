#include "disassembler.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(s, ...) check_(s, (uint8_t[]){__VA_ARGS__})

void check_(const char* inst, uint8_t* args) {
    char buf[MAX_DISASM_SZ];
    disassemble(args, buf);
    if (strcmp(buf, inst) != 0) {
        printf("Error. Expected '%s', found '%s'.\n", buf, inst);
        exit(1);
    }
}

int main()
{
    CHECK("nop", 0x0);
}

// vim:ts=4:sts=4:sw=4:expandtab
