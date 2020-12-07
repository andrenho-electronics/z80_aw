#include "memory.h"

#include <avr/io.h>

#include "bus.h"
#include "lowlevel.h"
#include "wait.h"
#include "z80.h"

uint16_t memory_read_addr()
{
    set_PL_165(1);   // load data into 74HC165
    set_PL_165(0);

    // read bits
    uint16_t addr = 0;
    for (int i = 15; i >= 0; --i) {
        addr <<= 1;
        addr |= get_MISO(); // retrieve bit
        set_SER_CLK(0);     // clock cycle
        set_SER_CLK(1);
    }

    return (addr << 8) | (addr >> 8);
}

uint8_t memory_read_data()
{
    DDRC = 0x0;
    return PINC;
}

/*static*/ void set_addr(uint16_t addr)  // TODO
{
    set_OE_595(1);
    set_SER_CLK(1);

    // send bits
    for (int i = 15; i >= 0; --i) {
        // feed data
        if (addr & (1 << i))
            set_SR_595(1);
        else
            set_SR_595(0);
        // cycle clock
        set_SER_CLK(0);     // clock cycle
        set_SER_CLK(1);
    }

    set_SER_CLK(0);     // clock cycle
    set_SER_CLK(1);

    // activate output
    set_OE_595(0);
}

uint16_t memory_read(uint16_t addr)
{
    if (z80_controls_bus())
        return -1;

    bus_mc_takeover();

    set_MREQ(1);
    set_WR(1);
    set_RD(1);

    set_addr(addr);
    wait();
    set_MREQ(0);
    set_RD(0);
    wait();

    uint8_t data = memory_read_data();
    wait();
    set_MREQ(1);
    set_RD(1);
    wait();

    bus_mc_release();  // this also put the ADDR pins in high impedance
    
    return data;
}

// vim:ts=4:sts=4:sw=4:expandtab
