#include "io.h"

#include <avr/io.h>
#include <avr/cpufunc.h>

#define OE_595  _BV(PORTB0)
#define SER_595 _BV(PORTB2)
#define Z80_CLK _BV(PORTB3)
#define Z80_RST _BV(PORTD5)
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

static uint8_t read_data()
{
    DDRC = 0x0;
    return PINC;
}

static uint16_t read_addr()
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

void io_init()
{
    DDRA  = 0x0;
    PORTA = 0x0;
    DDRB  = OE_595 | Z80_CLK;   // OE port of 595 needs to be held high (inactive)
                                //    for ADDR lines to be in high impedance
    PORTB = OE_595 | Z80_CLK;   // Z80 clock is initially held high
    DDRC  = 0x0;
    PORTC = 0x0;
    DDRD  = Z80_RST;   // when uC is turned on, Z80_RST is held low (active)
    PORTD = 0x0;
}

void io_read_inputs(uint16_t* addr, uint8_t* data, Inputs* in)
{
    *addr = read_addr();
    *data = read_data();
    in->mreq = PINA & MREQ;
    in->wr = PINA & WR;
    in->rd = PINA & RD;
    in->m1 = PIND & M1;
    in->iorq = PIND & IORQ;
    in->halt = PINA & HALT;
    in->busack = PIND & BUSACK;
}

void io_z80_clock()
{
    // TODO - request bus
    DDRD |= Z80_CLK;
    PORTB &= ~Z80_CLK;
    _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP();  // TODO - how many?
    PORTB |= Z80_CLK;
    _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP();
}

// vim:ts=4:sts=4:sw=4:expandtab
