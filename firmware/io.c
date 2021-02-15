#include "io.h"

#include <avr/io.h>

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
    }                                                      \
    static void set_ ## name ## _as_output()               \
    {                                                      \
        DDR ## port |= _BV(PORT ## port ## pin);           \
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
    }                                                      \
    static void set_ ## name ## _as_input()                \
    {                                                      \
        PORT ## port &= ~_BV(PORT ## port ## pin);         \
        DDR ## port &= ~_BV(PIN ## port ## pin);           \
    }
INPUT_PORTS
IO_PORTS
#undef P

//
// initialize ports
//

void io_init()
{
#define P(name, port, pin) set_ ## name ## _as_output();
    OUTPUT_PORTS
#undef P
#define P(name, port, pin) set_ ## name ## _as_input();
    INPUT_PORTS
    IO_PORTS
#undef P

    // TODO - move some of these to Z80
    set_OE_595(1);    // ADDR bus in high impedance
    set_SR_595(0);    // 595 serial waiting to begin transmitting
    set_SER_CLK(0);   // 165/595 clock wating to begin transmitting
    set_PL_165(1);    // not load data into 165
    set_ZRST(0);      // Z80 start in reset mode (active)
    set_ZCLK(0);      // Z80 clock waiting to begin transmitting
    set_BUSREQ(1);
    set_NMI(1);
    set_INT(1);
}

//
// control bus
//

bool memory_bus_takeover()
{
    if (p_ZRST == 1 && get_BUSACK() == 1)  // Z80 has control of the bus
        return false;
    set_MREQ_as_output();
    set_WR_as_output();
    set_RD_as_output();
    set_MREQ(1);
    set_WR(1);
    set_RD(1);
    return true;
}

void memory_bus_release()
{
    set_MREQ_as_input();
    set_WR_as_input();
    set_RD_as_input();
    set_OE_595(1);    // ADDR bus in high impedance
    DDRC = 0x0;       // DATA bus in high impedance
}

void data_bus_takeover()
{
    DDRC = 0xff;
}

void data_bus_release()
{
    DDRC = 0;
}

// vim:ts=4:sts=4:sw=4:expandtab
