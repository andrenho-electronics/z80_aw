#include "util.h"

unsigned int free_ram()
{
    extern int __heap_start, *__brkval;
    volatile int v;
    int free = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    return free;
}

// vim:ts=4:sts=4:sw=4:expandtab
