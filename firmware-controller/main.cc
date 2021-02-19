#include <avr/pgmspace.h>
#include <stdio.h>

#include "serial.hh"

int main()
{
    serial_init();
    printf_P(PSTR("Hello world!\n"));
}
