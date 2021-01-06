#include <stdint.h>

#include "z80/Z80.h"

extern uint8_t memory[64 * 1024];

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
    (void) Port; (void) Value;
}

byte InZ80(word Port)
{
    (void) Port;
    return 0;
}

word LoopZ80(Z80 *R)
{
    (void) R;
    return INT_QUIT;
}

void PatchZ80(Z80 *R)
{
    (void) R;
}
