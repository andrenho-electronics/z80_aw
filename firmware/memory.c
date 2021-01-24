#include "memory.h"

#include <limits.h>
#include <avr/io.h>
#include <util/delay.h>

#include "io.h"
#include "wait.h"

uint16_t memory_read_addr()
{
    set_PL_165(0);   // load data into 74HC165
    set_PL_165(1);

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

static void memory_set_addr(uint16_t addr)
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
        set_SER_CLK(0);
        set_SER_CLK(1);
    }

    set_SER_CLK(0);
    set_SER_CLK(1);

    // activate output
    set_OE_595(0);
}

void memory_set_data(uint8_t data)
{
    DDRC = 0xff;
    PORTC = data;
}

bool
memory_read_page(uint16_t addr, uint8_t* data, int count)
{
    if (memory_bus_takeover() == false)
        return false;

    set_MREQ(1);
    set_WR(1);
    set_RD(1);

    for (uint16_t a = 0; a < (size_t) count; ++a) {
        memory_set_addr(addr + a);
        wait();
        set_MREQ(0);
        set_RD(0);
        wait();
        wait();

        data[a] = memory_read_data();
        wait();
        set_MREQ(1);
        set_RD(1);
        wait();
    }

    // for some odd reason, when reading a memory block, the last byte is sometimes incorret.
    // Here we read the last byte again.
    memory_set_addr(addr + count - 1);
    wait();
    set_MREQ(0);
    set_RD(0);
    wait();
    wait();
    data[count - 1] = memory_read_data();
    wait();
    set_MREQ(1);
    set_RD(1);

    memory_bus_release();
    return true;
}

static void memory_write(uint16_t addr, uint8_t data, bool wait_for_completion)
{
    memory_set_addr(addr);
    memory_set_data(data);

    set_MREQ(0);
    set_WR(0);
    set_MREQ(1);
    set_WR(1);

    if (addr < 0x8000) {  // ROM
        if (wait_for_completion) {
            PORTC = 0;
            DDRC = 0;   // setup DATA bus for reading

            for (size_t i = 0; i < SIZE_MAX - 1; ++i) {
                set_RD(0);
                set_MREQ(0);
                uint8_t new_data = memory_read_data();
                set_MREQ(1);
                set_RD(1);
                if (new_data == data)
                    return;
            }
            _delay_ms(50);  // if still no match, wait a coniderable time
        }
    }
}

bool memory_write_page(uint16_t addr, uint8_t* data, size_t count, uint16_t* checksum)
{
    if (memory_bus_takeover() == false)
        return false;

    // write bytes
    for (uint16_t i = 0; i < count; ++i) {
        memory_write(addr + i, data[i], true);
    }
    memory_bus_release();
    
    // re-read data from memory
    memory_read_page(addr, data, count);

    // calculate checksum
    uint16_t checksum1 = 0, checksum2 = 0;
    for (size_t i = 0; i < count; ++i) {
        checksum1 = (checksum1 + data[i]) % 255;
        checksum2 = (checksum2 + checksum1) % 255;
    }

    *checksum = checksum1 | (checksum2 << 8);

    memory_bus_release();
    return true;
}

// vim:ts=4:sts=4:sw=4:expandtab
