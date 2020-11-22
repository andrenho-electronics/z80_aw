#include <string.h>

#include <util/delay.h>

#include "io.hh"
#include "computer.hh"
#include "serial.hh"
#include "repl.hh"

#include <avr/io.h>

int main()
{
    IO       io;
    Serial   serial;
    Computer computer(io);
    Repl     repl(serial, io);

    /*
    while (1) {
        computer.write_ram(0, 0xff);
        serial.printhex(computer.read_ram(0), 2); serial.puts();


        while (!( UCSRA & (1<<UDRE))); // Wait for empty transmit buffer
        UDR = '?';
        while (!( UCSRA & (1<<RXC)));  // wait for empty receive buffer
        volatile char c = UDR;
        _delay_ms(100);
    }
    */
    /*
    serial.printhex(computer.read_ram(0), 2); serial.puts();
    serial.printhex(computer.read_ram(1), 2); serial.puts();
    */

    waitk();
    computer.write_ram(1, 0xcd);

    while (1)
        ; //repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
