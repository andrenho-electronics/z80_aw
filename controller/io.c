#include "io.h"

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>

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

static void set_data(uint8_t data)
{
    DDRC = 0xff;
    PORTC = data;
}

static void set_addr(uint16_t addr)
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


void io_init()
{
    DDRA  = 0x0;
    PORTA = 0x0;
    DDRB  = OE_595 | Z80_CLK;   // OE port of 595 needs to be held high (inactive)
                                //    for ADDR lines to be in high impedance
    PORTB = OE_595;             // Z80 clock is initially held low
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
    PORTB |= Z80_CLK;
    _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP();  // TODO - how many?
    PORTB &= ~Z80_CLK;
    _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP();
}

void io_z80_init()
{
    PORTD &= ~Z80_RST;
    for (int i = 0; i < 10; ++i)
        io_z80_clock();
    PORTD |= Z80_RST;
}

void io_z80_reset()
{
    PORTD &= ~Z80_RST;
}

uint8_t io_read_memory(uint16_t addr)
{
    DDRC = 0;
    DDRA |= MREQ | WR | RD;

    PORTA = MREQ | WR | RD;
    _NOP(); _NOP(); _NOP();
    set_addr(addr);
    PORTA &= ~MREQ & ~RD;
    _NOP(); _NOP(); _NOP();

    uint8_t data = read_data();
    _NOP(); _NOP(); _NOP();
    PORTA |= MREQ | RD;

    DDRA &= ~MREQ & ~WR & ~RD;
    PORTB |= OE_595;  // OE port of 595 needs to be held high (inactive) for ADDR lines to be in high impedance

    return data;
}

void io_write_memory(uint16_t addr, uint8_t data)
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

    if (addr < 0x8000)  // ROM
        _delay_ms(10);

    DDRA &= ~MREQ & ~WR & ~RD;
    PORTB |= OE_595;   // OE port of 595 needs to be held high (inactive) for ADDR lines to be in high impedance
    DDRC = 0;
}

// vim:ts=4:sts=4:sw=4:expandtab
