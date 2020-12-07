#include "z80.h"

#include "bus.h"
#include "lowlevel.h"
#include "memory.h"
#include "wait.h"

Status   z80_last_status;
uint32_t z80_cycle_number = 0;

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
    if (Z.mreq == 0 && (Z.rd == 0 || Z.wr == 0)) {
        Z.addr_bus = memory_read_addr();
        Z.data_bus = memory_read_data();
    }
#undef Z 
}

bool z80_controls_bus()
{
    if (z80_last_status.busack == 0 || z80_last_status.reset == 0)
        return false;
    return true;
}

void z80_clock_cycle()
{
    set_ZCLK(1);
    wait();
    set_ZCLK(0);
    wait();
    update_status();
    if (z80_last_status.reset == 1)
        ++z80_cycle_number;
}

// vim:ts=4:sts=4:sw=4:expandtab
