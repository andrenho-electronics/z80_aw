#ifndef Z80_H_
#define Z80_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t cycle;
    int32_t  addr_bus;
    int16_t  data_bus;
    bool     m1, iorq, halt, busack;         // z80 output lines
    bool     wait, int_, nmi, reset, busreq; // z80 input lines
    bool     mreq, rd, wr;                   // memory lines
} Status;

bool z80_controls_bus();

void z80_powerdown();
void z80_init();
void z80_clock_cycle(bool request_bus);
void z80_bus_request();
void z80_keypress(uint8_t key, bool output_debugging_info);
void z80_update_status();

extern Status   z80_last_status;
extern uint16_t z80_last_pc;

#endif

// vim:ts=4:sts=4:sw=4:expandtab
