#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <stdbool.h>
#include <stdint.h>

void debugger_step(bool show_cycles, bool simple);
int  debugger_show_instructions(uint16_t addr);

void      debugger_clear_breakpoints();
void      debugger_add_breakpoint(uint16_t addr);
uint16_t* debugger_breakpoint_list(uint8_t* count);
void      debugger_continue();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
