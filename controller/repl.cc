#include "repl.hh"

#include <util/delay.h>

#define BUFSZ 256

Repl::Repl(Serial const& serial, IO const& io) 
    : serial(serial), io(io)
{
}

void
Repl::welcome() const
{
    serial.clear_screen();
    serial.puts("Welcome to Z80 controller.");
    serial.puts();
    print_instructions();
}

void
Repl::print_instructions() const
{
    serial.puts("Keys: [H]elp / [C]ycle / [R]ead memory / [W]rite memory");
    serial.puts("      Tests: [A]ddr / R[O]M");
    serial.puts();
    serial.puts("CYCLE    DATA ADDR MREQ WR RD M1 IORQ HALT BUSACK\a");
}

void
Repl::execute()
{
    io.set_high_impedance();
    print_input();

    while (true) {
        switch (serial.getc()) {
            case 'A': case 'a':
                test_addr();
                break;
            case 'O': case 'o':
                test_rom();
                break;
            case 'C': case 'c':
                ++cycle;
                return;
            case 'R': case 'r':
                read_memory();
                break;
            case 'W': case 'w':
                set_memory();
                break;
            case 'H': case 'h': case '?':
                print_instructions();
                break;
            default:
                serial.putc('\a');
        }
    }
}

void
Repl::print_input() const
{
    Inputs in = io.read_inputs();
    serial.printhex(cycle, 8);
    serial.print("  ");
    serial.printhex(in.data, 2);
    serial.print("  ");
    serial.printhex(in.addr, 4);
    serial.print("   ");
    serial.printbit(in.mreq);
    serial.print("   ");
    serial.printbit(in.wr);
    serial.print("  ");
    serial.printbit(in.rd);
    serial.print("  ");
    serial.printbit(in.m1);
    serial.print("   ");
    serial.printbit(in.iorq);
    serial.print("    ");
    serial.printbit(in.halt);
    serial.print("     ");
    serial.printbit(in.busack);
    serial.puts();
}

void
Repl::test_addr() const
{
    bool ok = true;
    for (uint32_t i = 0; i <= 0xfff0; i += 0x20) {
        serial.printhex(i, 4);
        serial.print("  ");
        for (uint8_t j = 0; j < 32; ++j) {
            uint16_t val = i + j;
            io.set_addr(val);
            if (io.read_addr() != val) {
                serial.printbit(0);
                ok = false;
            } else {
                serial.putc('.');
            }
        }
        serial.puts();
    }
    io.set_high_impedance();
    serial.puts(ok ? "Addr test ok." : "Some tests failed.");
    serial.puts();
    print_instructions();
}

void
Repl::test_rom() const
{
    bool ok = true;
    for (uint32_t k = 0; k < 0x8000; k += 0x7f00) {
        for (uint32_t i = 0; i <= 0xf0; i += 0x20) {
            serial.printhex(i + k, 4);
            serial.print("  ");
            for (uint8_t j = 0; j < 32; ++j) {
                uint16_t val = i + j + k;
                io.set_rom(val, val % 0xff);
                _delay_ms(12);
                if (io.read_mem(val) != (val % 0xff)) {
                    serial.printbit(0);
                    ok = false;
                } else {
                    serial.putc('.');
                }
            }
            serial.puts();
        }
    }
    io.set_high_impedance();
    serial.puts(ok ? "Addr test ok." : "Some tests failed.");
    serial.puts();
    print_instructions();
}

void
Repl::read_memory() const
{
    serial.print("Read memory location (in hex)? ");
    uint16_t addr = serial.gethex() / 0x100 * 0x100;
    for (int a = addr; a < (addr + 0x100); a += 0x10) {
        serial.printhex(a, 4);
        serial.print("  ");
        for (int b = 0; b < 0x10; ++b) {
            uint8_t v = io.read_mem(a + b);
            serial.printhex(v, 2);
            serial.print(" ");
        }
        serial.puts();
    }
}

void
Repl::set_memory() const
{
    serial.print("Write to which memory location (in hex)? ");
    uint16_t addr = serial.gethex();
    serial.print("Which data? ");
    uint8_t data = serial.gethex();
    io.set_rom(addr, data);
    _delay_ms(12);
    serial.puts("Ok.");
}

// vim:ts=4:sts=4:sw=4:expandtab
