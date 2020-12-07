#include "wait.h"

#include <util/delay.h>

#include "serial.h"

void wait()
{
    _delay_ms(1);
}

void waitk()
{
    serial_send('?');
    (volatile char) serial_recv();
}

// vim:ts=4:sts=4:sw=4:expandtab
