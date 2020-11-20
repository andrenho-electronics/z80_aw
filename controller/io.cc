#include "io.hh"

#include <avr/io.h>
#include <util/delay.h>

IO::IO()
{
    set_state(IO::State::HighImpedance);
}

void
IO::set_state(IO::State state) const
{
    switch (state) {
        case IO::State::HighImpedance:
        case IO::State::Input:
            DDRA = 0x0;
            DDRB = 0x0;
            DDRC = 0x0;
            DDRD = 0x0;
            PORTA = 0x0;
            PORTB = 0x0;
            PORTC = 0x0;
            PORTD = 0x0;
            break;
    }
}

Status
IO::read_status() const
{
    Status status;
    
    set_state(IO::State::Input);
    status.data = PINC;
    status.addr = read_addr();
    status.reg = read_parallel();
    set_state(IO::State::HighImpedance);

    return status;
}

uint16_t
IO::read_addr() const
{
    return (uint16_t) PINA | ((uint16_t) ((PINB << 5) | (PIND >> 3)) << 8);
}

uint8_t
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

    return parallel;

#undef SERIAL_IN
#undef LOAD
#undef CLOCK
}

// vim:ts=4:sts=4:sw=4:expandtab
