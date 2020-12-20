#include "debugger.h"

#include "ansi.h"
#include "disassembler.h"
#include "lowlevel.h"
#include "memory.h"
#include "repl.h"
#include "serial.h"
#include "z80.h"

#define DEBUG_INSTRUCTION_COUNT 8
#define MAX_BREAKPOINTS 16

static uint16_t breakpoints[MAX_BREAKPOINTS];
static uint8_t  breakpoint_count = 0;

#if ADD_DEBUGGER

static void show_registers(uint16_t addr)
{
    serial_printstr(PSTR("PC: " ANSI_GREEN));
    serial_printhex16(addr);
    serial_printstr(PSTR(ANSI_RESET));
    serial_puts();
}

static void show_stack()
{
}

static int debugger_print_instruction(uint16_t addr)
{
    serial_send('[');
    serial_printhex16(addr);
    serial_printstr(PSTR("] : "));

    uint8_t data[MAX_INST_SZ];
    char buf[MAX_DISASM_SZ];
    for (int i = 0; i < MAX_INST_SZ; ++i)
        data[i] = memory_read(addr + i);
    int r = disassemble(data, buf, NO_PREFIX);
    serial_print(buf);
    serial_puts();
    return r;
}

int debugger_show_instructions(uint16_t addr)
{
    for (int i = 0; i < DEBUG_INSTRUCTION_COUNT; ++i) {
        if (addr == z80_last_pc)
            serial_printstr(PSTR("-> "));
        else
            serial_spaces(3);
        addr += debugger_print_instruction(addr);
    }
    return addr;
}

#else

static void debugger_show_simple(uint16_t addr, uint8_t data)
{
    serial_send('[');
    serial_printhex16(addr);
    serial_printstr(PSTR("] = "));
    serial_printhex8(data);
    serial_puts();
}

#endif

void debugger_step(bool show_cycles, bool simple)
{
    if (get_ZRST() == 0) {
        serial_putsstr(PSTR("Z80 is powered down."));
        return;
    }

    if (!simple)
        serial_printstr(PSTR("---------------------\r\n"));

    bool busack = 1, m1 = 1;

    // run cycle until M1
    while (m1 == 1) {
        z80_clock_cycle(false);
        m1 = get_M1();
        if (show_cycles && !simple)
            repl_status();
    }
    uint16_t addr = z80_last_status.addr_bus;
    uint8_t data = z80_last_status.data_bus;
    (void) data;

    // run cycle until BUSACK
    while (busack == 1) {
        z80_clock_cycle(true);
        busack = get_BUSACK();
        if (show_cycles)
            repl_status();
    }

    // show debugging information
#if ADD_DEBUGGER
    if (!simple) {
        show_registers(addr);
        show_stack();
        debugger_show_instructions(addr);
    }
#endif
    if (simple) {
#if ADD_DEBUGGER
        debugger_print_instruction(addr);
#else
        debugger_show_simple(addr, data);
#endif
    }
}

void debugger_clear_breakpoints()
{
    breakpoint_count = 0;
}

uint16_t* debugger_breakpoint_list(uint8_t* count)
{
    *count = breakpoint_count;
    return breakpoints;
}

void debugger_add_breakpoint(uint16_t addr)
{
    if (breakpoint_count < MAX_BREAKPOINTS)
        breakpoints[breakpoint_count++] = addr;
}

void debugger_continue()
{
    while (1) {
        debugger_step(false, true);
        for (uint8_t i = 0; i < breakpoint_count; ++i) {
            if (z80_last_status.addr_bus == breakpoints[i]) {
                serial_printstr(PSTR("Breakpoint hit!\r\n"));
                return;
            }
        }
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
