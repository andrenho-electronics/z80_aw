#include "serial.h"

#include <avr/io.h>

#include <stdio.h>

static int uart_putchar(char c, FILE* f)
{
    (void) f;
    while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
    UDR = c;
    return 0;
}

static int uart_getchar(FILE* f)
{
    (void) f;
    while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
    return UDR;
}

void serial_init()
{
    // set baud rate - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    // int ubrr = 12;     // 38400 at 8 Mhz
    // int ubrr = 25;     // 38400 at 16 Mhz
    // int ubrr = 8;     // 115200 at 16 Mhz
    int ubrr = 26;       // 38400 at 16.5 Mhz
    UBRRH = (ubrr>>8);
    UBRRL = (ubrr);

    // set config
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);   // Async-mode 
    UCSRB = (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter

    static FILE uart = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
    stdout = &uart;
}

void
serial_send(uint8_t byte)
{
    while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
    UDR = byte;
}

uint8_t
serial_recv()
{
    while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
    return UDR;
}

uint8_t
serial_recv_noblock()
{
    if (UCSRA & (1<<RXC))
        return UDR;
    else
        return 0;
}

/*
void
serial_print(char* buf)
{
    while (*buf != '\0')
        serial_send(*buf++);
}

int
serial_printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int sz = vsprintf(NULL, fmt, ap);
    char* buf = malloc(sz + 1);
    vsnprintf(buf, sz, fmt, ap);
    serial_print(buf);
    free(buf);

    va_end(ap);
    return 0;
}
*/

// vim:ts=4:sts=4:sw=4:expandtab
