#ifndef Z80_H_
#define Z80_H_

#include <stdint.h>

typedef enum { E_NO_EVENT, E_BREAKPOINT_HIT, E_PRINT_CHAR } Z80_Event;
typedef enum { M_DEBUG, M_CONTINUE } Z80_Mode;

typedef struct {
    uint16_t af, bc, de, hl, afx, bcx, dex, hlx, ir, ix, iy, sp;
    uint8_t  halt;
} Z80_Registers;

// getter / setter
uint16_t      z80_pc();
Z80_Event     z80_last_event();
Z80_Mode      z80_mode();
uint8_t       z80_last_printed_char();
void          z80_set_last_keypress(uint8_t k);
Z80_Registers const* z80_registers_last_update();

// I/O
void    z80_interrupt(uint8_t vector);

// power
void    z80_powerdown();
void    z80_reset();

// step
uint8_t z80_step();
void    z80_step_debug();

// continue
void    z80_continue();
void    z80_stop();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
