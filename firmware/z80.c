#include "z80.h"

#include <stddef.h>

#include "memory.h"
#include "io.h"

static uint16_t pc = 0;
uint8_t last_printed_char = 0;
uint8_t last_keypress = 0;

uint16_t z80_pc()
{
    return pc;
}

inline static void z80_clock()
{
    set_ZCLK(1);
    set_ZCLK(0);
}

static void z80_out(uint16_t addr, uint8_t data)
{
    if ((addr & 0xff) == 0x0) {     // video OUT (print char)
        last_printed_char = data;
    }
}

static uint8_t z80_in(uint16_t addr)
{
    if ((addr & 0xff) == 0x1) {     // keyboard IN (last key pressed)
        return last_keypress;
    }
    return 0;
}

inline static void z80_check_iorq()
{
    if (get_IORQ() == 0) {
        uint16_t addr = memory_read_addr();
        if (get_WR() == 0) {
            uint8_t data = memory_read_data();
            z80_out(addr, data);
        } else if (get_RD() == 0) {
            data_bus_takeover();
            memory_set_data(z80_in(addr));
        }
        while (get_IORQ() == 0)
            z80_clock();
        data_bus_release();
    }
}

void z80_powerdown()
{
    set_ZRST(0);
    z80_clock();
}

void z80_reset()
{
    pc = 0;

    set_ZRST(0);

    for (int i = 0; i < 50; ++i)
        z80_clock();

    set_ZRST(1);
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
        z80_check_iorq();
        m1 = get_M1();
    }
    pc = memory_read_addr();

    // run cycle until BUSACK
    set_BUSREQ(0);
    while (busack == 1) {
        z80_clock();
        z80_check_iorq();
        busack = get_BUSACK();
    }
    set_BUSREQ(1);

    uint8_t c = last_printed_char;
    last_printed_char = 0;
    return c;
}

void z80_set_last_keypress(uint8_t k)
{
    last_keypress = k;
}

void z80_interrupt(uint8_t vector)
{
    set_INT(0);
    while (get_IORQ() == 1)
        z80_clock();
    data_bus_takeover();
    memory_set_data(vector);
    while (get_IORQ() == 0)
        z80_clock();
    data_bus_release();
}

// vim:ts=4:sts=4:sw=4:expandtab
