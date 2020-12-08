#include "repl.h"

#include "ansi.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"

static bool last_was_status = false;

static void repl_help()
{
    serial_puts("Help not implemented yet. See `repl.c`.");
}

static void repl_status()
{
#define Z z80_last_status
    if (!last_was_status) {
        serial_spaces(17);
        serial_puts("/- Z80 outputs --\\  /----- Z80 inputs -----\\  / memory \\");
        serial_puts("CYCLE ADDR DATA  M1 IORQ HALT BUSAK  WAIT INT NMI RSET BUSREQ  MREQ RD WR");
    } else {
        serial_print(ANSI_UP);
    }

    serial_print(ANSI_MAGENTA);
    serial_printhex16(z80_cycle_number);
    serial_print(ANSI_RESET);
    serial_spaces(2);

    if (Z.addr_bus >= 0)
        serial_printhex16(Z.addr_bus);
    else
        serial_print("----");

    serial_spaces(2);
    if (Z.data_bus >= 0) {
        if (Z.wr == 0)
            serial_print(ANSI_RED);
        serial_printhex8(Z.data_bus);
        serial_print(ANSI_RESET);
    } else {
        serial_print("--");
    }

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

    last_was_status = true;
#undef Z
}

static bool check_bus_control()
{
    if (z80_controls_bus()) {
        serial_puts("Z80 is in control of the bus.");
        return false;
    }
    return true;
}

static uint8_t print_data(uint16_t addr)
{
    serial_send('[');
    serial_printhex16(addr);
    serial_print("] = ");
    uint8_t data = memory_read(addr);
    serial_printhex8(data);
    serial_puts();
    return data;
}

static void repl_read_memory()
{
    if (!check_bus_control())
        return;
    serial_print("Address? ");
    uint16_t addr = serial_inputhex(4);
    print_data(addr);
}

static void repl_write_memory()
{
    if (!check_bus_control())
        return;
    serial_print("Address? ");
    uint16_t addr = serial_inputhex(4);
    serial_print("Data? ");
    uint8_t data = serial_inputhex(2);
    memory_write(addr, data);
    uint8_t new_data = print_data(addr);
    if (data != new_data)
        serial_puts(ANSI_RED "Data write failed." ANSI_RESET);
}

static void repl_dump_memory()
{
    if (!check_bus_control())
        return;
    serial_print("Page (0x100) ? ");
    uint8_t page = serial_inputhex(2);

    // read data
    uint8_t data[0x100];
    memory_read_page(page, data);

    // print header
    serial_puts(ANSI_MAGENTA "       _0 _1 _2 _3 _4 _5 _6 _7  _8 _9 _A _B _C _D _E _F");
        
    // print data
    for (uint16_t a = 0x0; a < 0x100; a += 0x10) {
        serial_print(ANSI_MAGENTA);
        serial_printhex16(((uint16_t) page * 0x100) + (a * 0x10));
        serial_print(ANSI_RESET);
        serial_spaces(3);
        for (uint16_t b = a; b < (a + 0x10); ++b) {
            serial_printhex8(data[b]);
            serial_send(' ');
            if (b - a == 7)
                serial_send(' ');
        }
        serial_spaces(2);
        for (uint16_t b = a; b < (a + 0x10); ++b)
            serial_send(data[b] >= 32 && data[b] < 127 ? data[b] : '.');
        serial_puts();
    }
}

static void repl_programatic_upload()
{
    if (z80_controls_bus())
        serial_send(1);
    else
        serial_send(0);
    uint16_t length = serial_recv16();
    for (uint16_t i = 0; i < length; ++i) {
        memory_write(i, serial_recv());
        serial_send(0);
    }
}

static void repl_programatic_download()
{
    if (z80_controls_bus())
        serial_send(1);
    else
        serial_send(0);
    uint16_t length = serial_recv16();
    for (uint16_t i = 0; i < length; ++i)
        serial_send(memory_read(i));
}

static void repl_powerdown()
{
    z80_powerdown();
    serial_puts("Z80 powered down.");
}

static void repl_init_z80()
{
    z80_init();
    serial_puts("Z80 (re)initialized.");
    last_was_status = false;
    repl_status();
}

void repl_exec()
{
    serial_print("(z80) \a");

    uint8_t c = serial_recv();
    if ((c > 32 && c < 127) || c == 13 || c == 10) {
        serial_send(c);
        serial_puts();
    }

    switch (c) {
        case 'h': repl_help(); break;
        case 's': repl_status(); break;
        case 'r': repl_read_memory(); break;
        case 'd': repl_dump_memory(); break;
        case 'w': repl_write_memory(); break;
        case 'p': repl_powerdown(); break;
        case 'i': repl_init_z80(); break;
        case 'b':
            z80_bus_request();
            serial_puts("Bus requested.");
            repl_status();
            break;
        case 'c': 
            z80_clock_cycle();
            repl_status();
            break;
        case '\n': case '\r':
            break;
        case PROGRAMATIC_UPLOAD: repl_programatic_upload(); break;
        case PROGRAMATIC_DOWNLOAD: repl_programatic_download(); break;
        default:
            serial_puts("Invalid command. Type 'h' for help.");
            break;
    }

    if (c != 's' && c != 'c' && c != 'i')
        last_was_status = false;
}

// vim:ts=4:sts=4:sw=4:expandtab
