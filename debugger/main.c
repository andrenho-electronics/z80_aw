#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

int main()
{
    // initialize UART

    // set speed
    int ubrr = 12;  // 9600 at 8 Mhz - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    UBRRH = (ubrr>>8);
    UBRRL = (ubrr);

    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);   // Async-mode 
    UCSRB = (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter

    for (;;) {
        while (!( UCSRA & (1<<UDRE))); /* Wait for empty transmit buffer*/
        UDR = 'H';
        while (!( UCSRA & (1<<UDRE))); /* Wait for empty transmit buffer*/
        UDR = '!';
        // for(;;) ;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
