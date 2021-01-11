#ifndef Z80_H_
#define Z80_H_

#include <stdint.h>

uint16_t z80_pc();

void    z80_powerdown();
void    z80_reset();
void    z80_set_last_keypress(uint8_t k);
void    z80_interrupt(uint8_t vector);
uint8_t z80_step();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
