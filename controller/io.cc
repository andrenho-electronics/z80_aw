#include "io.hh"

#include <avr/io.h>

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
            // address
            DDRA = 0x0;
            DDRB &= ~0b00000111;
            DDRC &= ~0b11111000;
            PORTA = 0x0;
            PORTB &= ~0b00000111;
            PORTC &= ~0b11111000;
            // data
            DDRC = 0x0;
            PORTC = 0x0;
            break;
    }
}

Status
IO::get_status() const
{
    Status status;
    
    set_state(IO::State::Input);
    status.data = PINC;
    status.addr = get_addr();
    set_state(IO::State::HighImpedance);

    return status;
}

uint16_t
IO::get_addr() const
{
    return (uint16_t) PINA | ((uint16_t) ((PINB << 5) | (PIND >> 3)) << 8);
}

// vim:ts=4:sts=4:sw=4:expandtab
