#include "serial.hh"

#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

static char hex(uint8_t v)
{
    return v < 10 ? v + '0' : v - 10 + 'A';
}

Serial::Serial()
{
    // set baud rate
    // int ubrr = 12;  // 38400 at 8 Mhz - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    int ubrr = 25;  // 38400 at 16 Mhz - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    UBRRH = (ubrr>>8);
    UBRRL = (ubrr);

    // set config
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);   // Async-mode 
    UCSRB = (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter

    // clear screen
    putc('\x1b'); putc('['); putc('2'); putc('J');
    putc('\x1b'); putc('['); putc('H');
}

void
Serial::putc(char c) const
{
    while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
    UDR = c;
}

char
Serial::getc() const
{
    while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
    return UDR;
}

void
Serial::getline(char* buf, size_t sz, bool echo) const
{
    size_t i = 0;
    do {
        while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
        if (i < sz) {
            buf[i++] = UDR;
            if (echo)
                putc(buf[i-1]);
        }
    } while (buf[i-1] != '\r');
    if (echo)
        putc('\n');
}

void
Serial::print(const char* text) const
{
    char* ptr = const_cast<char*>(text);
    while (*ptr)
        putc(*ptr++);
}

void
Serial::puts(const char* text) const
{
    print(text);
    putc('\r');
    putc('\n');
}

void
Serial::printhex(uint16_t value, uint8_t sz) const
{
    for (uint16_t i = sz; i > 0; --i)
        putc(hex((value >> ((i-1) * 4)) & 0xf));
}

void
waitk()
{
    while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
    UDR = '?';
    while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
    volatile char c = UDR;
    _delay_ms(100);
}

// vim:ts=4:sts=4:sw=4:expandtab
