#ifndef Z80_H_
#define Z80_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int32_t addr_bus;
    int16_t data_bus;
    bool    m1, iorq, halt, busack;         // z80 output lines
    bool    wait, int_, nmi, reset, busreq; // z80 input lines
    bool    mreq, rd, wr;                   // memory lines
} Status;

bool z80_controls_bus();

void z80_powerdown();
void z80_init();
void z80_clock_cycle();
void z80_bus_request();

extern Status   z80_last_status;
extern uint16_t z80_cycle_number;

#endif

// vim:ts=4:sts=4:sw=4:expandtab
