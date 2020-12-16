#include "repl.h"

#include <util/delay.h>

#include "ansi.h"
#include "debugger.h"
#include "lowlevel.h"
#include "memory.h"
#include "programatic.h"
#include "serial.h"
#include "tests.h"
#include "z80.h"

static bool last_was_status = false;
static long last_listing_pc = -1;

static void repl_help()
{
    serial_printstr(PSTR("Information:\r\n   "));
    serial_printstr(PSTR(ANSI_GREEN "h" ANSI_RESET "elp  "));
    serial_printstr(PSTR(ANSI_GREEN "s" ANSI_RESET "tatus   "));
    serial_printstr(PSTR(ANSI_GREEN "f" ANSI_RESET "ree RAM (controller)\r\n"));
    serial_printstr(PSTR("Memory:\r\n   "));
    serial_printstr(PSTR(ANSI_GREEN "r" ANSI_RESET "ead memory  "));
    serial_printstr(PSTR(ANSI_GREEN "d" ANSI_RESET "ump memory  "));
    serial_printstr(PSTR(ANSI_GREEN "w" ANSI_RESET "rite to memory\r\n"));
    serial_printstr(PSTR("Z80:\r\n   "));
    serial_printstr(PSTR(ANSI_GREEN "p" ANSI_RESET "owerdown  "));
    serial_printstr(PSTR(ANSI_GREEN "i" ANSI_RESET "nitialize (reset)  "));
    serial_printstr(PSTR(ANSI_GREEN "b" ANSI_RESET "us request  "));
    serial_printstr(PSTR(ANSI_GREEN "c" ANSI_RESET "ycle\r\n"));
    serial_printstr(PSTR("Devices:\r\n   "));
    serial_printstr(PSTR(ANSI_GREEN "k" ANSI_RESET "eyboard\r\n"));
    serial_printstr(PSTR("Debugger:\r\n   "));
    serial_printstr(PSTR("st" ANSI_GREEN "e" ANSI_RESET "p   "));
    serial_printstr(PSTR(ANSI_GREEN "l" ANSI_RESET "ist   "));
    serial_printstr(PSTR(ANSI_GREEN "L" ANSI_RESET "ist from addr  \r\n"));
#if ADD_TESTS
    serial_printstr(PSTR("Tests:\r\n   "));
    serial_printstr(PSTR("run " ANSI_GREEN "t" ANSI_RESET "ests\r\n"));
#endif
}

void repl_status()
{
#define Z z80_last_status
    if (!last_was_status) {
        serial_spaces(17);
        serial_putsstr(PSTR("/- Z80 outputs --\\  /----- Z80 inputs -----\\  / memory \\"));
        serial_putsstr(PSTR("CYCLE ADDR DATA  M1 IORQ HALT BUSAK  WAIT INT NMI RSET BUSREQ  MREQ RD WR"));
    }

    serial_printstr(PSTR(ANSI_MAGENTA));
    serial_printhex16(z80_cycle_number);
    serial_printstr(PSTR(ANSI_RESET));
    serial_spaces(2);

    if (Z.addr_bus >= 0)
        serial_printhex16(Z.addr_bus);
    else
        serial_printstr(PSTR("----"));

    serial_spaces(2);
    if (Z.data_bus >= 0) {
        if (Z.wr == 0)
            serial_printstr(PSTR(ANSI_RED));
        serial_printhex8(Z.data_bus);
        serial_printstr(PSTR(ANSI_RESET));
    } else {
        serial_printstr(PSTR("--"));
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
        serial_putsstr(PSTR("Z80 is in control of the bus."));
        return false;
    }
    return true;
}

static uint8_t print_data(uint16_t addr)
{
    serial_send('[');
    serial_printhex16(addr);
    serial_printstr(PSTR("] = "));
    uint8_t data = memory_read(addr);
    serial_printhex8(data);
    serial_puts();
    return data;
}

static void repl_read_memory()
{
    if (!check_bus_control())
        return;
    serial_printstr(PSTR("Address? "));
    uint16_t addr = serial_inputhex(4);
    print_data(addr);
}

static void repl_write_memory()
{
    if (!check_bus_control())
        return;
    serial_printstr(PSTR("Address? "));
    uint16_t addr = serial_inputhex(4);
    serial_printstr(PSTR("Data? "));
    uint8_t data = serial_inputhex(2);
    memory_write(addr, data, true);
    uint8_t new_data = print_data(addr);
    if (data != new_data)
        serial_putsstr(PSTR(ANSI_RED "Data write failed." ANSI_RESET));
}

static void repl_dump_memory()
{
    if (!check_bus_control())
        return;
    serial_printstr(PSTR("Page (0x100) ? "));
    uint8_t page = serial_inputhex(2);

    // read data
    uint8_t data[0x100];
    memory_read_page(page * 0x100, data, 0x100);

    // print header
    serial_spaces(7);
    serial_putsstr(PSTR(ANSI_MAGENTA "_0 _1 _2 _3 _4 _5 _6 _7  _8 _9 _A _B _C _D _E _F"));
        
    // print data
    for (uint16_t a = 0x0; a < 0x100; a += 0x10) {
        serial_printstr(PSTR(ANSI_MAGENTA));
        serial_printhex16(((uint16_t) page * 0x100) + a);
        serial_printstr(PSTR("\b_" ANSI_RESET));
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

static void repl_list(bool ask_addr)
{
    uint16_t pc = z80_last_pc;
    if (ask_addr) {
        serial_printstr(PSTR("Address? "));
        pc = serial_inputhex(4);
    } else if (last_listing_pc >= 0) {
        pc = last_listing_pc;
    }
    last_listing_pc = debugger_show_instructions(pc);
}

static void repl_powerdown()
{
    z80_powerdown();
    serial_putsstr(PSTR("Z80 powered down."));
}

static void repl_init_z80()
{
    z80_init();
    serial_putsstr(PSTR("Z80 (re)initialized."));
}

void repl_free_ram()
{
    serial_printint(free_ram());
    serial_printstr(PSTR(" bytes free."));
    serial_puts();
}

static void repl_keyboard()
{
    serial_printstr(PSTR("Key? "));
    uint8_t key = serial_recv();
    serial_printhex8(key);
    serial_puts();
    z80_keypress(key);
}

void repl_exec()
{
    (void) repl_list;

    uint8_t c = serial_recv();

    switch (c) {
        case 'h': repl_help(); break;
        case 's': repl_status(); break;
        case 'r': repl_read_memory(); break;
        case 'd': repl_dump_memory(); break;
        case 'w': repl_write_memory(); break;
        case 'p': repl_powerdown(); break;
        case 'i': repl_init_z80(); break;
        case 'f': repl_free_ram(); break;
        case 'e': debugger_step(false); break;
        case 'E': debugger_step(true); break;
#if ADD_DEBUGGER
        case 'l': repl_list(false); break;
        case 'L': repl_list(true); break;
#endif
        case 'k': repl_keyboard(); break;
#if ADD_TESTS
        case 't': tests_run(); break;
#endif
        case 'b':
            z80_clock_cycle(true);
            repl_status();
            break;
        case 'c': 
            z80_clock_cycle(false);
            repl_status();
            break;
        case 0xC:  // Ctrl+L
            serial_printstr(PSTR(ANSI_CLRSCR));
            break;
        case 0xfe:
            programatic_upload();
            break;
        case '\n': case '\r':
            break;
        default:
            break;
    }

    if (free_ram() < 200)
        serial_printstr(PSTR("** Warning, < 200 bytes free."));

    if (c != 's' && c != 'c' && c != 'b')
        last_was_status = false;
    if (c != 'l' && c != 'L')
        last_listing_pc = -1;
}

// vim:ts=4:sts=4:sw=4:expandtab
