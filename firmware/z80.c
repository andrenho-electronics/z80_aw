#include "z80.h"

#include "bus.h"
#include "lowlevel.h"
#include "memory.h"
#include "wait.h"

Status   z80_last_status;
uint16_t z80_cycle_number = 0;
uint16_t z80_last_pc = 0;

static void update_status()
{
#define Z z80_last_status
    bus_mc_release();
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

#include "serial.h"

static void z80_iorq_requested()
{
    serial_printstr(PSTR("IORQ requested by a device!\n"));
}

static void z80_clock()
{
    set_ZCLK(1);
    wait();
    set_ZCLK(0);
    wait();
    if (z80_last_status.iorq == 1 && get_IORQ() == 0) {
        z80_iorq_requested();
    }
    update_status();
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
}

void z80_init()
{
    bus_mc_release();

    set_ZRST(0);
    z80_cycle_number = 0;
    wait();

    for (int i = 0; i < 50; ++i)
        z80_clock_cycle(false);

    set_ZRST(1);
    wait();
    z80_clock_cycle(false);
}

// vim:ts=4:sts=4:sw=4:expandtab
