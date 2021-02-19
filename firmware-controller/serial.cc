#include "serial.hh"

#include <stdio.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

Serial
Serial::init()
{
    // set baud rate - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    // int ubrr = 12;     // 38400 at 8 Mhz
    // int ubrr = 25;     // 38400 at 16 Mhz
    int ubrr = 8;     // 115200 at 16 Mhz
    // int ubrr = 26;       // 38400 at 16.5 Mhz
    UBRRH = (ubrr>>8);
    UBRRL = (ubrr);

    // set config
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);   // Async-mode 
    UCSRB = (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter

    // static FILE uart = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
    static FILE uart;
    uart.put = [](char c, FILE*) {
        while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
        UDR = c;
        if (c == '\n') {
            while (!( UCSRA & (1<<UDRE)));
            UDR = '\r';
        }
        return 0;
    };
    uart.get = [](FILE*) -> int {
        while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
        return UDR;
    };
    uart.flags = _FDEV_SETUP_RW;
    stdout = stdin = &uart;

    return Serial();
}

void
Serial::send(uint8_t byte) const
{
    while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
    UDR = byte;
}

uint8_t
Serial::recv() const
{
    while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
    return UDR;
}

uint8_t
Serial::recv_noblock() const
{
    if (UCSRA & (1<<RXC))
        return UDR;
    else
        return 0;
}

void
Serial::clrscr() const
{
    printf_P(PSTR("\e[1;1H\e[2J"));  // clear screen
}
