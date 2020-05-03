#include "serial.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

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

char ser_input(unsigned* data1, unsigned* data2)
{
    char buf[24] = { 0 };
    unsigned i = 0;
    do {
        while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
        if (i < sizeof buf) {
            buf[i++] = UDR;
            ser_printchar(buf[i-1]);
        }
    } while (buf[i-1] != '\r');
    ser_printchar('\n');

    char command = 0;
    sscanf(buf, "%c %x %x", &command, data1, data2);  // TODO - do this manually if there's no space in uc
    
    return command;
}

// vim:ts=4:sts=4:sw=4:expandtab
