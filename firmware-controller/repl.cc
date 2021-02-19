#include "repl.hh"

#include <avr/pgmspace.h>
#include <stdio.h>

static unsigned int free_ram()
{
    extern int __heap_start, *__brkval;
    volatile int v;
    int free = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    return free;
}

void repl_do()
{
    char cmd;
    if (scanf("%c", &cmd) == 0)
        return;
    switch (cmd) {
        case 'f':
            printf_P(PSTR("%d bytes free.\n"), free_ram());
            break;
    }
}
