#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <stdbool.h>
#include <stdint.h>

void debugger_step(bool show_cycles);
int  debugger_show_instructions(uint16_t addr);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
