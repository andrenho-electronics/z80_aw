#include "debugger.h"

#include "lowlevel.h"
#include "repl.h"
#include "serial.h"
#include "z80.h"

void debugger_step()
{
    if (get_ZRST() == 0) {
        serial_putsstr(PSTR("Z80 is powered down."));
        return;
    }

    bool busack = 1, m1 = 1;

    // run cycle until M1
    while (m1 == 1) {
        z80_clock_cycle(false);
        m1 = get_M1();
    }
    uint16_t addr = z80_last_status.addr_bus;
    uint8_t  data = z80_last_status.data_bus;

    // run cycle until BUSACK
    while (busack == 1) {
        z80_clock_cycle(true);
        busack = get_BUSACK();
    }

    serial_printstr(PSTR("PC: "));
    serial_printhex16(addr);
    serial_printstr(PSTR(" -> "));
    serial_printhex8(data);
    serial_puts();
}

// vim:ts=4:sts=4:sw=4:expandtab
