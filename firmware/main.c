#include <avr/interrupt.h>
#include <avr/io.h>

#include "dbg.h"
#include "io.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"

typedef enum {
    V_NO_EVENT, V_SERIAL_IN,
} Event;
static Event next_event = E_NO_EVENT;

int main()
{
    io_init();

    // setup RX interrupt
    serial_init();

    // setup INT0 interrupt (for IORQ - video)
    GICR |= (1 << INT0);    // enable interrupt 0
    MCUCR &= ~(1 << ISC00);
    MCUCR |= (1 << ISC01);   // on falling edge
    sei();

    // main loop
    for (;;) {
        if (z80_mode() == M_CONTINUE)
            z80_step();
        switch (next_event) {
            case V_SERIAL_IN:
                cli();
                debugger_cycle();
                sei();
                break;
        }
        next_event = V_NO_EVENT;
    }
}

ISR(INT0_vect)   // fired on IRQ falling edge
{
    cli();

    sei();
}

ISR(USART_RXC_vect)    // fired on receiving input from serial
{
    next_event = V_SERIAL_IN;
}

// vim:ts=4:sts=4:sw=4:expandtab
