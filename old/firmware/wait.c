#include "wait.h"

#include <avr/cpufunc.h>
#include <util/delay.h>

#include "serial.h"

void wait()
{
    // _delay_ms(1);
    _NOP();
    _NOP();
    _NOP();
    _NOP();
    _NOP();
    _NOP();
}

void waitk()
{
    serial_send('?');
    (volatile char) serial_recv();
}

// vim:ts=4:sts=4:sw=4:expandtab
