#include "z80.h"

#include "bus.h"
#include "lowlevel.h"
#include "memory.h"
#include "serial.h"

#include "wait.h"

Status   z80_last_status;
uint16_t z80_last_pc = 0;

static uint8_t  last_key_pressed = 0;
static uint8_t  last_printed_char = 0;
static uint16_t z80_cycle_number = 0;

typedef struct {
    bool    fired;
    uint8_t command;
} Interrupt;
Interrupt interrupt = { .fired = false, .command = 0 };

static void z80_clock();

void z80_update_status()
{
#define Z z80_last_status
    bus_mc_release();
    Z.cycle = z80_cycle_number;
    Z.addr_bus = -1;
    Z.data_bus = -1;
    Z.m1 = get_M1();
    Z.iorq = get_IORQ();
    Z.halt = get_HALT();
    Z.busack = get_BUSACK();
    Z.wait = get_WAIT();
    Z.int_ = get_INT();
    Z.nmi = get_NMI();
    Z.reset = get_ZRST();
    Z.busreq = get_BUSREQ();
    Z.mreq = get_MREQ();
    Z.rd = get_RD();
    Z.wr = get_WR();
    if (Z.rd == 0 || Z.wr == 0) {
        Z.addr_bus = memory_read_addr();
        Z.data_bus = memory_read_data();
        if (Z.m1 == 0)
            z80_last_pc = Z.addr_bus;
    }
#undef Z 
}

bool z80_controls_bus()
{
    if (z80_last_status.busack == 0 || z80_last_status.reset == 0)
        return false;
    return true;
}

static void z80_iorq_requested()
{
    if (get_MREQ() == 1 && get_RD() == 1) {  // it's IORQ from an interrupt
        memory_set_data(interrupt.command);
        z80_clock();
        interrupt.fired = false;
    }

    uint16_t addr = memory_read_addr();
    uint8_t data = memory_read_data();
    if ((addr & 0xff) == 0x00) {   // video device
        last_printed_char = data;
    } else if ((addr & 0xff) == 0x01) {  // last keyboard press
        memory_set_data(last_key_pressed);
        z80_clock();
        z80_last_status.data_bus = last_key_pressed;
    }
}

static void z80_clock()
{
    set_INT(!interrupt.fired);
    set_ZCLK(1);
    wait();
    set_ZCLK(0);
    wait();
    bool last_iorq = z80_last_status.iorq;
    z80_update_status();
    if (last_iorq == 1 && get_IORQ() == 0) {
        z80_iorq_requested();
    }
    if (z80_last_status.reset == 1)
        ++z80_cycle_number;
}

void z80_clock_cycle(bool request_bus)
{
    bus_mc_release();
    set_BUSREQ(!request_bus);
    z80_clock();
}

void z80_powerdown()
{
    set_ZRST(0);
    z80_cycle_number = 0;
    z80_clock_cycle(false);
    bus_mc_takeover();
}

void z80_init()
{
    bus_mc_release();

    set_ZRST(0);
    z80_cycle_number = 0;
    z80_last_pc = 0;
    wait();

    for (int i = 0; i < 50; ++i)
        z80_clock_cycle(false);

    set_ZRST(1);
    wait();
    z80_step();
}

void z80_keypress(uint8_t key)
{
    last_key_pressed = key;
    interrupt = (Interrupt) { .fired = true, .command = 0xcf };

    /*
    bus_mc_release();

    // fire interrupt
    set_INT(0);
    set_BUSREQ(1);
    for (int i = 0; i < 15; ++i) {
        z80_clock();
        if (get_IORQ() == 0)
            goto z80_response;
    }
    set_INT(1);   // a interrupt request was not accepted by Z80
    return;

z80_response:
    set_INT(1);

    do {
        memory_set_data(0xcf);
        z80_clock();
        z80_last_status.data_bus = 0xcf;
    } while (get_IORQ() == 0);
    */
}

uint8_t z80_step()
{
    bool busack = 1, m1 = 1;

    // run cycle until M1
    while (m1 == 1) {
        z80_clock_cycle(false);
        m1 = get_M1();
    }
    uint16_t addr = z80_last_status.addr_bus;
    z80_last_pc = addr;

    // run cycle until BUSACK
    while (busack == 1) {
        z80_clock_cycle(true);
        busack = get_BUSACK();
    }

    uint8_t c = last_printed_char;
    last_printed_char = 0;
    return c;
}

// vim:ts=4:sts=4:sw=4:expandtab
