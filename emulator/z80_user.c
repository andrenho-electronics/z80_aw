#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "z80/Z80.h"

extern uint8_t memory[64 * 1024];
extern uint8_t last_printed_char;
extern uint8_t last_keypress;
extern bool    keyboard_interrupt;

void WrZ80(word Addr,byte Value)
{
    memory[Addr] = Value;
}

byte RdZ80(word Addr)
{
    return memory[Addr];
}

void OutZ80(word Port,byte Value)
{
    if ((Port & 0xff) == 0x0)  // video
        last_printed_char = Value;
}

byte InZ80(word Port)
{
    if ((Port & 0xff) == 0x1)  // keyboard
        return last_keypress;
    return 0;
}

word LoopZ80(Z80 *R)
{
    (void) R;
    if (keyboard_interrupt) {
        keyboard_interrupt = false;
        return 0xcf;  // rst 0x8, returned by the keyboard controller
    }
    return INT_QUIT;
}

void PatchZ80(Z80 *R)
{
    (void) R;
}
