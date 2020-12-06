#include "memory.h"

#include <avr/io.h>

#include "lowlevel.h"

uint16_t memory_read_addr()
{
    set_PL_165(1);   // load data into 74HC165
    set_PL_165(0);

    // read bits
    uint16_t addr = 0;
    for (int i = 15; i >= 0; --i) {
        addr <<= 1;
        addr |= get_MISO(); // retrieve bit
        set_CLK_165(0);     // clock cycle
        set_CLK_165(1);
    }

    return (addr << 8) | (addr >> 8);
}

uint8_t  memory_read_data()
{
    DDRC = 0x0;
    return PINC;
}

// vim:ts=4:sts=4:sw=4:expandtab
