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
    if (value) {
        DDRA = 0; PORTA = 0;
        DDRB = 0; PORTB = 0;
        DDRC = 0; PORTC = 0;
        DDRD = 0b10; PORTD = 0;  // PD1 = TX
    } else {
        DDRA = 0xff;
        DDRB = 0b1111;  // TODO - PB pins 4..7
        DDRC = 0xff;
        DDRD = 0b11111110;  // PD0 = RX
        PORTA |= _BV(PA7);  // CE = 1
        PORTB |= _BV(PB3) | _BV(PB2);  // OE = WE = 1
    }
}

static void
set_CE(uint8_t value)
{
    if (value)
        PORTA |= _BV(PA7);
    else
        PORTA &= ~_BV(PA7);
}

static void
set_OE(uint8_t value)
{
    if (value)
        PORTB |= _BV(PB3);
    else
        PORTB &= ~_BV(PB3);
}

static void
set_WE(uint8_t value)
{
    if (value)
        PORTB |= _BV(PB2);
    else
        PORTB &= ~_BV(PB2);
}

static void
set_addr(uint16_t addr)
{
    PORTC = addr & 0xff;
    PORTA &= ~0b111111;
    PORTA |= ((addr >> 8) & 0b1111111);
}

static void
set_data(uint8_t value)
{
    DDRB |= 0b00000011;
    DDRD |= 0b11111100;
    _delay_us(1);
    PORTB &= ~0b00000011;
    PORTB |= (value & 0b11);
    PORTD &= ~0b11111100;
    PORTD |= (value & 0b11111100);
}

static uint8_t
get_data()
{
    DDRB  &= ~0b00000011;
    DDRD  &= ~0b11111100;
    _delay_us(1);
    PORTB &= ~0b00000011;
    PORTD &= ~0b11111100;
    uint8_t v = PINB & 0b11;
    v |= (PIND & 0b11111100);
    return v;
}

// TODO - temp
void
io_test()
{
    set_highZ(false);
    set_OE(1);
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
    return data;
}

void
io_write(uint16_t addr, uint8_t data)
{
    // write address (WE controlled)
    set_highZ(false);
    set_CE(0);
    set_addr(addr);
    set_WE(0);
    set_data(data);
    _delay_us(1);
    set_WE(1);
    _delay_us(1);
    set_CE(1);
    set_highZ(true);
}

// vim:ts=4:sts=4:sw=4:expandtab
