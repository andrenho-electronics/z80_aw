#ifndef Z80_H_
#define Z80_H_

#include <stdint.h>

typedef enum { E_NO_EVENT, E_BREAKPOINT_HIT } Z80_Event;
typedef enum { M_DEBUG, M_CONTINUE } Z80_Mode;

uint16_t  z80_pc();
Z80_Event z80_last_event();
Z80_Mode  z80_mode();

void    z80_powerdown();
void    z80_reset();
void    z80_set_last_keypress(uint8_t k);
void    z80_interrupt(uint8_t vector);
uint8_t z80_step();
void    z80_continue();
void    z80_stop();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
