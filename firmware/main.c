#include <avr/interrupt.h>
#include <avr/io.h>

#include "dbg.h"
#include "io.h"
#include "serial.h"

typedef enum {
    E_NO_EVENT, E_SERIAL_IN,
} Event;

static Event next_event = E_NO_EVENT;

int main()
{
    io_init();

    // setup RX interrupt
    serial_init();
    UCSRB |= (1 << RXEN) | (1 << RXCIE);

    // setup INT0 interrupt (for IORQ - video)
    GICR |= (1 << INT0);    // enable interrupt 0
    MCUCR &= ~(1 << ISC00);
    MCUCR |= (1 << ISC01);   // on falling edge
    sei();

    // main loop
    for (;;) {
        switch (next_event) {
            case E_NO_EVENT:
                break;
            case E_SERIAL_IN:
                debugger_cycle();
                break;
        }
        next_event = E_NO_EVENT;
    }
}

ISR(INT0_vect)   // fired on IRQ falling edge
{
    cli();
    sei();
}

ISR(USART_RXC_vect)    // fired on receiving input from serial
{
    next_event = E_SERIAL_IN;
}

// vim:ts=4:sts=4:sw=4:expandtab
