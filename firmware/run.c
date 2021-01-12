#include "run.h"

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "io.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"

#define CLK_UP   0b00001011
#define CLK_DOWN 0b00000011

void run()
{
    // setup RX interrupt
    UCSRB |= (1 << RXEN) | (1 << RXCIE);

    // setup INT0 interrupt (for IORQ - video)
    GICR |= (1 << INT0);     // enable interrupt 0
    MCUCR &= ~(1 << ISC00);
    MCUCR |= (1 << ISC01);   // on falling edge
    sei();

    z80_reset();

    // TODO - replace by an AVR timer 
    // (see https://www.reddit.com/r/arduino/comments/3898g6/generating_14_mhz_clock_on_arduino/)
    for (;;) {
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
    }
}

ISR(INT0_vect)
{
    cli();
    z80_check_iorq();
    uint8_t c = z80_last_printed_char();
    z80_clear_last_printed_char();
    if (c != 0)
        serial_send(c);
    sei();
}

ISR(USART_RXC_vect)
{
    cli();
    z80_set_last_keypress(serial_recv());
    z80_interrupt(0xcf);   // RST 0x8  
    sei();
}

// vim:ts=4:sts=4:sw=4:expandtab
