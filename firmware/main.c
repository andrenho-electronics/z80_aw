#include <avr/interrupt.h>
#include <avr/io.h>

#include "dbg.h"
#include "io.h"
#include "memory.h"
#include "serial.h"

typedef enum {
    E_NO_EVENT, E_SERIAL_IN,
} Event;
static Event next_event = E_NO_EVENT;

uint8_t last_printed_char = 0;

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
        switch (next_event) {
            case E_NO_EVENT:
                break;
            case E_SERIAL_IN:
                cli();
                debugger_cycle();
                sei();
                break;
        }
        next_event = E_NO_EVENT;
    }
}

ISR(INT0_vect)   // fired on IRQ falling edge
{
    cli();

    uint16_t addr = memory_read_addr();

    if ((addr & 0xff) == 0x00) {   // video device
        uint8_t data = addr >> 8;
        last_printed_char = data;
        serial_send(0xff);
    } else if ((addr & 0xff) == 0x01) {   // retrieve last key pressed
        /*
        memory_set_data(last_key_pressed);
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
        */
    }

    sei();
}

ISR(USART_RXC_vect)    // fired on receiving input from serial
{
    next_event = E_SERIAL_IN;
}

// vim:ts=4:sts=4:sw=4:expandtab
