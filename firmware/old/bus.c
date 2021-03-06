#include "bus.h"

#include <avr/io.h>

#include "lowlevel.h"

void bus_init()
{
    bus_mc_release();

    // these pins will always be controlled by the MC
    set_OE_595(1);    // ADDR bus in high impedance
    set_SR_595(0);    // 595 serial waiting to begin transmitting
    set_SER_CLK(0);   // 165/595 clock wating to begin transmitting
    set_PL_165(1);    // not load data into 165
    set_ZCLK(0);      // Z80 clock waiting to begin transmitting
    set_ZRST(0);      // Z80 start in reset mode (active)
    set_BUSREQ(1);
    set_NMI(1);

    // these pins will always be read-only
    set_IORQ(X);
    set_BUSACK(X);
    set_M1(X);
    set_X1(X);
    set_HALT(X);
}

void bus_mc_takeover()
{
    set_MREQ(1);
    set_WR(1);
    set_RD(1);
    set_INT(1);
    set_WAIT(1);
}

void bus_mc_release()
{
    set_MREQ(X);
    set_WR(X);
    set_RD(X);
    set_WAIT(X);
    set_OE_595(1);    // ADDR bus in high impedance
    DDRC = 0x0;       // DATA bus in high impedance
}

// vim:ts=4:sts=4:sw=4:expandtab
