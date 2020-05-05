#include "io.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <stdbool.h>

/* ports:
 *   atmega - at28c256
 *   PA0..6 - A8-A14
 *   PA7    - CE
 *   PB0..1 - D0..1
 *   PB2    - WE
 *   PB3    - OE
 *   PB4..7 - NC
 *   PC0..7 - A0..7
 *   PD0..1 - serial RX/TX
 *   PD2..7 - D2..7 */

static void
set_highZ(bool value)
{
}

static void
set_CE(uint8_t value)
{
}

static void
set_OE(uint8_t value)
{
}

static void
set_WE(uint8_t value)
{
}

static void
set_addr(uint16_t addr)
{
}

static void
set_data(uint8_t addr)
{
}

static uint8_t
get_data()
{
    return 0;
}

void
io_init()
{
    set_highZ(true);
}

uint8_t
io_read(uint16_t addr)
{
    set_highZ(false);
    set_addr(addr);
    set_CE(0);
    set_OE(0);
    _delay_us(1);
    uint8_t data = get_data();
    set_CE(1);
    set_OE(1);
    _delay_us(1);
    set_highZ(true);
    return 0;
}

void
io_write(uint16_t addr, uint8_t data)
{
    // write address (WE controlled)
    set_highZ(false);
    set_CE(0);
    set_WE(0);
    set_addr(addr);
    _delay_us(1);
    set_data(data);
    _delay_us(1);
    set_WE(1);
    set_CE(1);
    set_highZ(true);
}

// vim:ts=4:sts=4:sw=4:expandtab
