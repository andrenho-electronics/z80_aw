#include "lowlevel.h"

#include <avr/io.h>

#define P(name, port, pin)                              \
    bool get_ ## name()                                 \
    {                                                   \
        set_ ## name(X);                                \
        return PIN ## pin & _BV(P ## port ## pin);      \
    }                                                   \
                                                        \
    void set_ ## name(uint8_t v)                        \
    {                                                   \
        switch (v) {                                    \
            case L:                                     \
                DDR ## port |= _BV(P ## port ## pin);   \
                PORT ## port &= ~_BV(P ## port ## pin); \
                break;                                  \
            case H:                                     \
                DDR ## port |= _BV(P ## port ## pin);   \
                PORT ## port |= _BV(P ## port ## pin);  \
                break;                                  \
            default:                                    \
                PORT ## port &= ~_BV(P ## port ## pin); \
                DDR ## port &= ~_BV(P ## port ## pin);  \
                break;                                  \
        }                                               \
    }
LIST_OF_PORTS
#undef P

// vim:ts=4:sts=4:sw=4:expandtab
