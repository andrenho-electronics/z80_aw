#ifndef LOWLEVEL_H_
#define LOWLEVEL_H_

#include <stdbool.h>
#include <stdint.h>

#define LIST_OF_PORTS   \
    P(OE_595, B, 0)     \
    P(PL_165, B, 1)     \
    P(SR_595, B, 2)     \
    P(ZCLK,   B, 3)     \
    P(MISO,   B, 6)     \
    P(IORQ,   D, 2)     \
    P(BUSACK, D, 3)     \
    P(M1,     D, 4)     \
    P(ZRST,   D, 5)     \
    P(X1,     D, 6)     \
    P(SER_CLK,D, 7)     \
    P(MREQ,   A, 0)     \
    P(RD,     A, 1)     \
    P(WR,     A, 2)     \
    P(HALT,   A, 3)     \
    P(WAIT,   A, 4)     \
    P(INT,    A, 5)     \
    P(NMI,    A, 6)     \
    P(BUSREQ, A, 7)

#define L 0
#define H 1
#define X 2

#define P(name, port, pin) \
    bool get_ ## name();   \
    void set_ ## name(uint8_t v);
LIST_OF_PORTS
#undef P

#endif

// vim:ts=4:sts=4:sw=4:expandtab
