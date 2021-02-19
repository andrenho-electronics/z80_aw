#include <avr/pgmspace.h>
#include <stdio.h>

#include "repl.hh"
#include "serial.hh"

Serial serial = Serial::init();

int main()
{
    serial.clrscr();
    while (true)
        repl_do();
}
