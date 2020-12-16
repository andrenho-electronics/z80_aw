#include "run.h"

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "lowlevel.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"

#define CLK_UP   0b00001011
#define CLK_DOWN 0b00000011

static uint8_t last_key_pressed;

void run()
{
    serial_putsstr(PSTR("Welcome to the Z80-AW computer.\r\n"));

    // setup RX interrupt
    UCSRB |= (1 << RXEN) | (1 << RXCIE);

    // setup INT0 interrupt (for IORQ - video)
    GICR |= (1 << INT0);    // enable interrupt 0
    MCUCR |= (1 << ISC01);   // on falling edge
    MCUCR &= ~(1 << ISC00);
    sei();

    z80_init();

    // TODO - replace by an AVR timer
    for (;;) {
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
    }
}

ISR(INT0_vect)
{
    uint16_t addr = memory_read_addr();
    if ((addr & 0xff) == 0x00) {   // video device
        uint8_t data = addr >> 8;
        serial_send(data);
    }
}

ISR(USART_RXC_vect)
{
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
    return;

z80_response:
    set_INT(1);

    for (int i = 0; i < 3; ++i) {
        memory_set_data(0xcf);
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
