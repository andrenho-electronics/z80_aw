#include "io.hh"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/cpufunc.h>

#include "serial.hh"

#define OE_595  _BV(PORTB0)
#define SER_595 _BV(PORTB2)
#define Z80_CLK _BV(PORTB3)
#define MREQ    _BV(PORTA0)
#define WR      _BV(PORTA2)
#define RD      _BV(PORTA1)
#define M1      _BV(PORTD4)
#define IORQ    _BV(PORTD2)
#define HALT    _BV(PORTA4)
#define BUSACK  _BV(PORTD3)
#define PL_165  _BV(PORTB1)
#define MISO    _BV(PORTB6)
#define MOSI    _BV(PORTB5)
#define SER_CLK _BV(PORTD7)

void
IO::set_high_impedance() const
{
    DDRA  = 0x0;
    PORTA = 0x0;
    DDRB  = OE_595 | Z80_CLK;   // OE port of 595 and Z80 clock needs to be held high
    PORTB = OE_595 | Z80_CLK;
    DDRC  = 0x0;
    PORTC = 0x0;
    DDRD  = 0x0;
    PORTD = 0x0;
}

Inputs
IO::read_inputs() const
{
    Inputs in;
    in.data = read_data();
    in.addr = read_addr();
    in.mreq = PINA & MREQ;
    in.wr = PINA & WR;
    in.rd = PINA & RD;
    in.m1 = PIND & M1;
    in.iorq = PIND & IORQ;
    in.halt = PINA & HALT;
    in.busack = PIND & BUSACK;
    return in;
}

uint8_t
IO::read_data() const
{
    DDRC = 0x0;
    return PINC;
}

void
IO::set_data(uint8_t data) const
{
    DDRC = 0xff;
    PORTC = data;
}

uint16_t
IO::read_addr() const
{
    DDRB |= PL_165;   // outputs
    DDRD |= SER_CLK;
    DDRB &= ~MISO;    // inputs
    PORTB |= PL_165;
    PORTD |= SER_CLK;

    // load 165 with data
    PORTB &= ~PL_165;
    PORTB |= PL_165;

    // read bits
    uint16_t addr = 0;
    for (int i = 15; i >= 0; --i) {
        addr <<= 1;
        addr |= (PINB >> PORTB6) & 1;
        PORTD &= ~SER_CLK;  // clock cycle
        PORTD |= SER_CLK;
    }
    
    // restore ports
    DDRB &= ~PL_165;
    DDRD &= ~SER_CLK;
    return (addr << 8) | (addr >> 8);
}

void
IO::set_addr(uint16_t addr) const
{
    // set ports for output
    DDRB |= SER_595 | OE_595;
    DDRD |= SER_CLK;
    PORTB |= OE_595;
    PORTD |= SER_CLK;

    // send bits
    for (int i = 15; i >= 0; --i) {
        // feed data
        if (addr & (1 << i))
            PORTB |= SER_595;
        else
            PORTB &= ~SER_595;
        // cycle clock
        PORTD &= ~SER_CLK;
        PORTD |= SER_CLK;
    }

    PORTD &= ~SER_CLK;
    PORTD |= SER_CLK;
    
    // activate output
    PORTB &= ~OE_595;

    // restore ports
    DDRB &= ~SER_595;
}

void
IO::set_rom(uint16_t addr, uint8_t data) const
{
    DDRA = MREQ | WR | RD;
    PORTA = MREQ | WR | RD;
    _NOP(); _NOP(); _NOP();

    set_addr(addr);
    set_data(data);
    _NOP(); _NOP(); _NOP();

    PORTA &= ~MREQ;
    PORTA &= ~WR;
    _NOP(); _NOP(); _NOP();
    _NOP(); _NOP(); _NOP();
    PORTA |= MREQ;
    _NOP(); _NOP(); _NOP();
    PORTA |= WR;
    _NOP(); _NOP(); _NOP();

    DDRC = 0;
}

uint8_t
IO::read_mem(uint16_t addr) const
{
    DDRA = MREQ | WR | RD;
    DDRC = 0;

    PORTA = MREQ | WR | RD;
    _NOP(); _NOP(); _NOP();
    set_addr(addr);
    PORTA &= ~MREQ & ~RD;
    _NOP(); _NOP(); _NOP();

    uint8_t data = read_data();
    _NOP(); _NOP(); _NOP();
    PORTA |= MREQ | RD;

    return data;
}

// vim:ts=4:sts=4:sw=4:expandtab