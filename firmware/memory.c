#include "memory.h"

#include <avr/io.h>
#include <util/delay.h>

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
    return PINC;
}

static void set_addr(uint16_t addr)
{
    set_OE_595(1);
    set_SER_CLK(0);

    // send bits
    for (int i = 15; i >= 0; --i) {
        // feed data
        if (addr & (1 << i))
            set_SR_595(1);
        else
            set_SR_595(0);
        // cycle clock
        set_SER_CLK(1);
        set_SER_CLK(0);     // clock cycle
    }

    set_SER_CLK(1);
    set_SER_CLK(0);     // clock cycle

    // activate output
    set_OE_595(0);
}

static void set_data(uint8_t data)
{
    DDRC = 0xff;
    PORTC = data;
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
    set_RD(1);
    set_MREQ(1);
    wait();

    bus_mc_release();  // this also put the ADDR pins in high impedance
    
    return data;
}

void
memory_read_page(uint8_t page, uint8_t data[0x100])
{
    if (z80_controls_bus()) {
        for (uint16_t a = 0; a < 0x100; ++a)
            data[a] = 0xff;
    }

    bus_mc_takeover();
    set_MREQ(1);
    set_WR(1);
    set_RD(1);

    for (uint16_t a = 0; a < 0x100; ++a) {
        uint16_t addr = (uint16_t) page * 0x100 + a;
        set_addr(addr);
        wait();
        set_MREQ(0);
        set_RD(0);
        wait();

        data[a] = memory_read_data();
        wait();
        set_MREQ(1);
        set_RD(1);
        wait();
    }

    bus_mc_release();  // this also put the ADDR pins in high impedance
}

void memory_write(uint16_t addr, uint8_t data)
{
    if (z80_controls_bus())
        return;

    bus_mc_takeover();
    set_MREQ(1);
    set_WR(1);
    set_RD(1);
    wait();

    set_addr(addr);
    set_data(data);
    wait();

    set_MREQ(0);
    set_WR(0);
    wait();
    set_MREQ(1);
    set_WR(1);
    wait();

    if (addr < 0x8000)  // ROM
        _delay_ms(10);

    bus_mc_release();  // this also put the ADDR & DATA pins in high impedance
}

// vim:ts=4:sts=4:sw=4:expandtab
