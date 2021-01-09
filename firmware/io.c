#include "io.h"

#include <avr/io.h>

static const bool IN  = 0,
                  OUT = 1;

//
// setters
//

#define P(name, port, pin)                                 \
    static bool p_ ## name = false;                        \
    void set_ ## name(bool v)                              \
    {                                                      \
        if (v)                                             \
            PORT ## port |= _BV(PORT ## port ## pin);      \
        else                                               \
            PORT ## port &= ~_BV(PORT ## port ## pin);     \
        p_ ## name = v;                                    \
    }
OUTPUT_PORTS
IO_PORTS
#undef P

// 
// getters
//

#define P(name, port, pin)                                 \
    bool get_ ## name()                                    \
    {                                                      \
        return PIN ## port & _BV(PIN ## port ## pin);      \
    }
INPUT_PORTS
IO_PORTS
#undef P

// 
// set_direction_XXXX
//

#define P(name, port, pin)                                 \
    static void set_direction_ ## name(bool dir)           \
    {                                                      \
        if (dir == IN)                                     \
            DDR ## port &= ~_BV(PIN ## port ## pin);       \
        else if (dir == OUT)                               \
            DDR ## port |= _BV(PORT ## port ## pin);       \
    }
INPUT_PORTS
OUTPUT_PORTS
IO_PORTS
#undef P

//
// initialize ports
//

void io_init()
{
#define P(name, port, pin) set_direction_ ## name(OUT);
    OUTPUT_PORTS
#undef P
#define P(name, port, pin) set_direction_ ## name(IN);
    INPUT_PORTS
    IO_PORTS
#undef P
}

//
// control bus
//

bool memory_bus_takeover()
{
    if (p_ZRST == 1 && get_BUSACK() == 1)  // Z80 has control of the bus
        return false;
#define P(name, port, pin) set_direction_ ## name(OUT);
    IO_PORTS
#undef P
    return true;
}

void memory_bus_release()
{
#define P(name, port, pin) set_direction_ ## name(OUT);
    IO_PORTS
#undef P
}

// vim:ts=4:sts=4:sw=4:expandtab
