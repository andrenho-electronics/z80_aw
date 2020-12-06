#include "repl.h"

#include "serial.h"
#include "z80.h"

static void repl_help()
{
    serial_puts("Help not implemented yet. See `repl.c`.");
}

static void repl_status()
{
#define Z z80_last_status
    serial_puts("           /- Z80 outputs --\\  /----- Z80 inputs -----\\  / memory \\");
    serial_puts("ADDR DATA  M1 IORQ HALT BUSAK  WAIT INT NMI RSET BUSREQ  MREQ RD WR");

    if (Z.addr_bus > 0)
        serial_printhex16(Z.addr_bus);
    else
        serial_print("----");

    serial_spaces(2);
    if (Z.data_bus > 0)
        serial_printhex8(Z.data_bus);
    else
        serial_print("--");

    serial_spaces(4);
    serial_printbit(Z.m1);

    serial_spaces(3);
    serial_printbit(Z.iorq);
    
    serial_spaces(4);
    serial_printbit(Z.halt);
    
    serial_spaces(5);
    serial_printbit(Z.busack);
    
    serial_spaces(5);
    serial_printbit(Z.wait);

    serial_spaces(3);
    serial_printbit(Z.int_);

    serial_spaces(3);
    serial_printbit(Z.nmi);

    serial_spaces(4);
    serial_printbit(Z.reset);

    serial_spaces(6);
    serial_printbit(Z.busreq);

    serial_spaces(5);
    serial_printbit(Z.mreq);

    serial_spaces(3);
    serial_printbit(Z.rd);

    serial_spaces(2);
    serial_printbit(Z.wr);

    serial_puts();
#undef Z
}

void repl_exec()
{
    serial_print("(z80) ");

    uint8_t c = serial_recv();
    if (c > 32 && c < 127)
        serial_send(c);
    serial_puts();

    switch (c) {
        case 'h': repl_help(); break;
        case 's': repl_status(); break;
        case 'c': 
            z80_clock_cycle();
            repl_status();
            break;
        default:
            serial_puts("Invalid command. Type 'h' for help.");
            break;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
