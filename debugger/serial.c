#include "serial.h"

#include <avr/interrupt.h>
#include <avr/io.h>

void ser_init()
{
    // set baud rate
    int ubrr = 12;  // 38400 at 8 Mhz - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    UBRRH = (ubrr>>8);
    UBRRL = (ubrr);

    // set config
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);   // Async-mode 
    UCSRB = (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter
}

void ser_printchar(char c)
{
    while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
    UDR = c;
}

void ser_printstr(const char* text)
{
    for (int i = 0; text[i]; ++i)
        ser_printchar(text[i]);
}

void ser_input()
{
    char c;
    do {
        while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
        c = UDR;
        ser_printchar(c);
    } while (c != '\r');
}

// vim:ts=4:sts=4:sw=4:expandtab
