#include "z80.h"

#include <stddef.h>

#include "breakpoints.h"
#include "memory.h"
#include "io.h"

static uint16_t  pc = 0;
static uint8_t   last_printed_char = 0;
static uint8_t   last_keypress = 0;
static int       last_interrupt = -1;
static Z80_Event last_event = E_NO_EVENT;
static Z80_Mode  mode = M_DEBUG;
static uint32_t  cycle_number = 0;

Z80_Event z80_last_event()
{
    Z80_Event e = last_event;
    last_event = E_NO_EVENT;
    return e;
}

Z80_Mode z80_mode()
{
    return mode;
}

uint16_t z80_pc()
{
    return pc;
}

uint8_t z80_last_printed_char()
{
    return last_printed_char;
}

static inline void z80_clock()
{
    set_ZCLK(1);
    set_ZCLK(0);
    ++cycle_number;
}

static void z80_out(uint16_t addr, uint8_t data)
{
    if ((addr & 0xff) == 0x0) {     // video OUT (print char)
        last_printed_char = data;
        if (mode == M_CONTINUE)
            last_event = E_PRINT_CHAR;
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
        if (get_WR() == 0) {  // OUT
            uint8_t data = memory_read_data();
            z80_out(addr, data);
        } else if (get_RD() == 0) {  // IN
            data_bus_takeover();
            memory_set_data(z80_in(addr));
        } else {  // INTERRUPT
            if (last_interrupt != -1) {
                data_bus_takeover();
                memory_set_data(last_interrupt);
                last_interrupt = -1;
                set_INT(1);
            }
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
    cycle_number = 0;

    set_BUSREQ(1);
    set_NMI(1);
    set_WAIT(1);
    set_INT(1);
    memory_bus_release();

    set_ZRST(0);

    for (int i = 0; i < 50; ++i)
        z80_clock();

    set_ZRST(1);
    z80_step();
}

inline static void z80_busreq()
{
    bool busack = 1;

    set_BUSREQ(0);
    while (busack == 1) {
        z80_clock();
        z80_check_iorq();
        busack = get_BUSACK();
    }
    set_BUSREQ(1);
}

uint8_t z80_step()
{
    if (mode == M_CONTINUE && last_event != E_NO_EVENT)
        return 0;  // we wait until the last event is consumed by the client

    bool m1 = 1;

    // run cycle until M1
    set_BUSREQ(1);
    while (m1 == 1) {
        z80_clock();
        z80_check_iorq();
        m1 = get_M1();
    }
    pc = memory_read_addr();

    switch (mode) {
        case M_DEBUG:
            // run cycle until BUSACK
            if (mode == M_DEBUG)
                z80_busreq();
            break;
        case M_CONTINUE:
            // find out if breakpoint was hit
            if (bkp_in_list(pc)) {
                last_event = E_BREAKPOINT_HIT;
                mode = M_DEBUG;
            }
            return 0;
    }

    uint8_t c = last_printed_char;
    last_printed_char = 0;
    return c;
}

void z80_continue()
{
    mode = M_CONTINUE;
}

void z80_stop()
{
    mode = M_DEBUG;
    z80_busreq();
}

void z80_set_last_keypress(uint8_t k)
{
    last_keypress = k;
}

void z80_interrupt(uint8_t vector)
{
    set_INT(0);
    last_interrupt = vector;
    z80_clock();
    z80_check_iorq();
}

// vim:ts=4:sts=4:sw=4:expandtab
