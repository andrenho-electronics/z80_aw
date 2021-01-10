#include "z80.h"

#include <stddef.h>

#include "io.h"
#include "wait.h"

static uint16_t pc = 0;

uint16_t z80_pc()
{
    return pc;
}

static void z80_clock()
{
    set_ZCLK(1);
    wait();
    set_ZCLK(0);
    wait();
}

void z80_powerdown()
{
    set_ZRST(0);
    z80_clock();
}

void z80_reset()
{
    set_ZRST(0);
    for (size_t i = 0; i < 16; ++i)
        z80_clock();
    set_ZRST(1);
    pc = 0;
    // TODO - step
}

// vim:ts=4:sts=4:sw=4:expandtab
