#include "run.h"

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "lowlevel.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"

#define CLK_UP   0b00001011
#define CLK_DOWN 0b00000011

static uint8_t last_key_pressed = 0;

void run()
{
    // setup RX interrupt
    UCSRB |= (1 << RXEN) | (1 << RXCIE);

    // setup INT0 interrupt (for IORQ - video)
    GICR |= (1 << INT0);    // enable interrupt 0
    MCUCR &= ~(1 << ISC00);
    MCUCR |= (1 << ISC01);   // on falling edge
    sei();

    serial_putsstr(PSTR("Z80-AW computer initialized.\r\n"));

    z80_init();  // improve this initialization (?)

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

    uint16_t addr = memory_read_addr();
    if ((addr & 0xff) == 0x00) {   // video device
        uint8_t data = addr >> 8;
        serial_send(data);
    } else if ((addr & 0xff) == 0x01) {   // retrieve last key pressed
        memory_set_data(last_key_pressed);
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
    }

    sei();

    DDRC = 0x0;
}

ISR(USART_RXC_vect)
{
    cli();

    last_key_pressed = serial_recv();

    // fire interrupt
    set_INT(0);
    set_BUSREQ(1);
    for (int i = 0; i < 15; ++i) {
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
        if (get_IORQ() == 0)
            goto z80_response;
    }
    set_INT(1);   // a interrupt request was not accepted by Z80
    sei();
    DDRC = 0x0;
    return;

z80_response:
    set_INT(1);

    do {
        memory_set_data(0xcf);
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
    } while (get_IORQ() == 0);

    sei();
    DDRC = 0x0;
}

// vim:ts=4:sts=4:sw=4:expandtab
