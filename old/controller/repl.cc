#include "repl.hh"

#define BUFSZ 256

void
Repl::execute()
{
    char buf[BUFSZ];
    serial.getline(buf, BUFSZ, true);
    if (buf[0] != 0) {
        Command command = parse_command(buf);
        switch (command.cmd) {
            case 'h':
                print_help();
                break;
            case 's':
                print_cpu_status();
                break;
            default:
                serial.puts("Syntax error.");
        }
    }
}

Repl::Command
Repl::parse_command(const char* str) const
{
    Repl::Command cmd { str[0] };
    // TODO
    return cmd;
}

void
Repl::print_help() const
{
    serial.puts("s - print CPU status");
}

void
Repl::print_cpu_status() const
{
    Status status = io.read_status();
    serial.print("ADDR  = "); serial.printhex(status.addr, 4); serial.puts();
    serial.print("DATA  = "); serial.printhex(status.data, 2); serial.puts();
    serial.print("halt  = "); serial.printhex(status.flags.halt, 1); serial.puts();
    serial.print("mreq  = "); serial.printhex(status.flags.mreq, 1); serial.puts();
    serial.print("ioreq = "); serial.printhex(status.flags.ioreq, 1); serial.puts();
    serial.print("m1    = "); serial.printhex(status.flags.m1, 1); serial.puts();
    serial.print("busak = "); serial.printhex(status.flags.busack, 1); serial.puts();
    serial.print("wr    = "); serial.printhex(status.flags.wr, 1); serial.puts();
    serial.print("rd    = "); serial.printhex(status.flags.rd, 1); serial.puts();
}

// vim:ts=4:sts=4:sw=4:expandtab
