#include "lowlevel.h"

#include <avr/io.h>

unsigned int free_ram()
{
    extern int __heap_start, *__brkval;
    volatile int v;
    int free = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    return free;
}

#define P(name, port, pin)                                 \
    bool get_ ## name()                                    \
    {                                                      \
        return PIN ## port & _BV(PIN ## port ## pin);      \
    }                                                      \
                                                           \
    void set_ ## name(uint8_t v)                           \
    {                                                      \
        switch (v) {                                       \
            case L:                                        \
                DDR ## port |= _BV(PORT ## port ## pin);   \
                PORT ## port &= ~_BV(PORT ## port ## pin); \
                break;                                     \
            case H:                                        \
                DDR ## port |= _BV(PORT ## port ## pin);   \
                PORT ## port |= _BV(PORT ## port ## pin);  \
                break;                                     \
            default:                                       \
                DDR ## port &= ~_BV(PIN ## port ## pin);   \
                break;                                     \
        }                                                  \
    }
LIST_OF_PORTS
#undef P

// vim:ts=4:sts=4:sw=4:expandtab
