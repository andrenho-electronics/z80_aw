#include "serial.h"

#include <avr/io.h>

void serial_init()
{
    // set baud rate - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    // int ubrr = 12;     // 38400 at 8 Mhz
    // int ubrr = 25;     // 38400 at 16 Mhz
    int ubrr = 8;     // 115200 at 16 Mhz
    UBRRH = (ubrr>>8);
    UBRRL = (ubrr);

    // set config
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);   // Async-mode 
    UCSRB = (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter

    serial_ei();
}

void
serial_send(uint8_t byte)
{
    while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
    UDR = byte;
}

void
serial_send16(uint16_t word)
{
    serial_send(word & 0xff);
    serial_send(word >> 8);
}

uint8_t
serial_recv()
{
    while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
    return UDR;
}

uint16_t serial_recv16()
{
    uint16_t r = serial_recv();
    r |= ((uint16_t) serial_recv()) << 8;
    return r;
}

void serial_ei()
{
    UCSRB |= (1 << RXEN) | (1 << RXCIE);
}

void serial_di()
{
    UCSRB &= ~(1 << RXEN);
    UCSRB &= ~(1 << RXCIE);
}

// vim:ts=4:sts=4:sw=4:expandtab
