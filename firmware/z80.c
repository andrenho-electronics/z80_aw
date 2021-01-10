#include "z80.h"

#include <stddef.h>

#include "memory.h"
#include "io.h"
#include "wait.h"

static uint16_t pc = 0;
extern uint8_t last_printed_char;

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
    wait();
}

void z80_reset()
{
    pc = 0;

    set_ZRST(0);
    wait();

    for (int i = 0; i < 50; ++i)
        z80_clock();

    set_ZRST(1);
    wait();
    z80_step();
}

uint8_t z80_step()
{
    memory_bus_release();

    bool busack = 1, m1 = 1;

    // run cycle until M1
    set_BUSREQ(1);
    while (m1 == 1) {
        z80_clock();
        m1 = get_M1();
    }
    pc = memory_read_addr();

    // run cycle until BUSACK
    set_BUSREQ(0);
    while (busack == 1) {
        z80_clock();
        busack = get_BUSACK();
    }
    set_BUSREQ(1);

    uint8_t c = last_printed_char;
    last_printed_char = 0;
    return c;
}

// vim:ts=4:sts=4:sw=4:expandtab
