#ifndef IO_H_
#define IO_H_

#include <stdbool.h>

void io_init();

bool memory_bus_takeover();
void memory_bus_release();

void data_bus_takeover();
void data_bus_release();

// ports only written by the microcontroller (set_XXX)
#define OUTPUT_PORTS    \
    P(OE_595, B, 0)     \
    P(PL_165, B, 1)     \
    P(SR_595, B, 2)     \
    P(ZCLK,   B, 3)     \
    P(ZRST,   D, 5)     \
    P(SER_CLK,D, 7)     \
    P(WAIT,   A, 4)     \
    P(INT,    A, 5)     \
    P(NMI,    A, 6)     \
    P(BUSREQ, A, 7)

// ports only read by the microcontroller (get_XXX)
#define INPUT_PORTS     \
    P(MISO,   B, 6)     \
    P(M1,     D, 4)     \
    P(X1,     D, 6)     \
    P(BUSACK, D, 3)     \
    P(IORQ,   D, 2)     \
    P(HALT,   A, 3)

// ports both read and written by the microcontroller (get_XXX, set_XXX)
#define IO_PORTS        \
    P(MREQ,   A, 0)     \
    P(RD,     A, 1)     \
    P(WR,     A, 2)

#define P(name, port, pin) \
    void set_ ## name(bool v);
OUTPUT_PORTS
IO_PORTS
#undef P

#define P(name, port, pin) \
    bool get_ ## name();
INPUT_PORTS
IO_PORTS
#undef P

#endif

// vim:ts=4:sts=4:sw=4:expandtab
