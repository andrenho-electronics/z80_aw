#include "io.hh"

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

IO::IO()
{
    set_high_impedance();
}

void
IO::set_high_impedance() const
{
#define OUTPUT_ENABLE PORTD2
    DDRA = 0x0;
    DDRB = 0x0;
    DDRC = 0x0;
    DDRD = _BV(OUTPUT_ENABLE);
    PORTA = 0x0;
    PORTB = 0x0;
    PORTC = 0x0;
    PORTD = _BV(OUTPUT_ENABLE);  // disable parallel output
#undef OUTPUT_ENABLE
}

Status
IO::read_status() const
{
    Status status;
    
    set_high_impedance();
    status.data = read_data();
    status.addr = read_addr();
    status.flags = read_parallel();

    return status;
}

uint8_t
IO::read_data() const
{
    DDRC = 0x0;
    return PINC;
}

uint16_t
IO::read_addr() const
{
    return (uint16_t) PINA | ((uint16_t) ((PINB << 5) | (PIND >> 3)) << 8);
}

CpuFlagsOut
IO::read_parallel() const
{
#define SERIAL_IN PORTB4
#define LOAD      PORTB5
#define CLOCK     PORTB7

    uint8_t parallel = 0;

    DDRB &= ~_BV(SERIAL_IN);          // set SERIAL IN as input
    DDRB |= _BV(LOAD) | _BV(CLOCK);   // set LOAD and CLOCK as output
    PORTB |= _BV(LOAD) | _BV(CLOCK);  // LOAD is disabled (=1)

    // load data into IC
    PORTB &= ~_BV(LOAD);
    PORTB |= _BV(LOAD);

    // read data
    for (int i = 0; i < 8; ++i) {
        parallel <<= 1;
        parallel |= (PINB >> SERIAL_IN) & 1;
        PORTB &= ~_BV(CLOCK);
        PORTB |= _BV(CLOCK);
    }

    CpuFlagsOut cfo;
    memcpy(&cfo, &parallel, 1);
    return cfo;

#undef SERIAL_IN
#undef LOAD
#undef CLOCK
}

void
IO::write_data(uint8_t data) const
{
    DDRC = 0xff;
    PORTC = data;
}

void
IO::write_addr(uint16_t data) const
{
    DDRA = 0xff;
    DDRD |= 0b11111000;
    DDRB |= 0b00000111;
    PORTA = data & 0xff;
    PORTD &= ~0b11111000;
    PORTD |= ((data >> 8) & 0b11111) << 3;
    PORTB &= ~0b111;
    PORTB |= data >> 13;
}

void
IO::write_flags(CpuFlagsIn flags) const
{
#define SERIAL_OUT    PORTB5
#define CLOCK         PORTB7
#define OUTPUT_ENABLE PORTD2

    uint8_t f = 0;
    memcpy(&f, &flags, 1);

    DDRB |= _BV(SERIAL_OUT) | _BV(CLOCK);
    DDRD |= _BV(OUTPUT_ENABLE);
    PORTD |= _BV(OUTPUT_ENABLE);   // output is disabled
    PORTB &= ~_BV(CLOCK);          // clock is low

    // send bits
    for (int i = 7; i >= 0; --i) {
        if (f & (1 << i))
            PORTB |= _BV(SERIAL_OUT);
        else
            PORTB &= ~_BV(SERIAL_OUT);
        PORTB |= _BV(CLOCK);
        PORTB &= ~_BV(CLOCK);
    }

    // enable output
    PORTD &= ~_BV(OUTPUT_ENABLE);

    // back to input (except OUTPUT_ENABLE)
    DDRB &= ~(_BV(SERIAL_OUT) | _BV(CLOCK));

#undef SERIAL_OUT
#undef CLEAR
#undef CLOCK
#undef OUTPUT_ENABLE
}

// vim:ts=4:sts=4:sw=4:expandtab
