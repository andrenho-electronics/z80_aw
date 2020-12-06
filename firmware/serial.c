#include "serial.h"

#include <avr/io.h>

#include "ansi.h"

void serial_init()
{
    // set baud rate - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    // int ubrr = 12;  // 38400 at 8 Mhz
    int ubrr = 25;     // 38400 at 16 Mhz
    UBRRH = (ubrr>>8);
    UBRRL = (ubrr);

    // set config
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);   // Async-mode 
    UCSRB = (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter
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

void
serial_print(const char* s)
{
    char* p = (char *) s;
    while (*p) {
        serial_send(*p);
        ++p;
    }
}

static void
serial_printhexdigit(uint8_t data)
{
    serial_send(data < 0xa ? data + '0' : data - 0xa + 'A');
}

void
serial_printhex8(uint8_t data)
{
    serial_printhexdigit(data >> 4);
    serial_printhexdigit(data & 0xf);
}

void
serial_printhex16(uint16_t data)
{
    serial_printhex8(data >> 8);
    serial_printhex8(data & 0xff);
}

void
serial_printbit(bool v)
{
    if (v)
        serial_print(ANSI_GREEN "1" ANSI_RESET);
    else
        serial_print(ANSI_RED "0" ANSI_RESET);
}

void
serial_spaces(int n)
{
    for (int i = 0; i < n; ++i)
        serial_send(' ');
}

uint16_t
serial_inputhex()
{
    char buf[5] = { 0, 0, 0, 0, 0 };
    uint8_t current = 0;
    
    while (1) {
        uint8_t c = serial_recv();
        if (c == '\n' || c == '\r')
            break;
        if (current < 4) {
            if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
                if (c >= 'a')
                    c -= 'a' - 'A';  // convert to uppercase
                buf[current++] = c;
                serial_send(c);
            }
        }
        if ((c == '\b' || c == 127) && current > 0) {
            buf[current--] = 0;
            serial_print("\b \b");
        }
    }

    serial_puts();
    
    // convert string to hex value
    current = 0;
    uint16_t ret = 0;
    while (buf[current]) {
        char v = buf[current];
        ret <<= 4;
        if (v >= '0' && v <= '9')
            ret |= (v - '0');
        else
            ret |= (v - 'A' + 0xa);
        ++current;
    }

    return ret;
}

// vim:ts=4:sts=4:sw=4:expandtab
