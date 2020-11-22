#include "computer.hh"

#include "serial.hh"

void
Computer::clear_bus() const
{
    // TODO
    io.set_high_impedance();
}

uint8_t
Computer::read_ram(uint16_t addr) const
{
    clear_bus();
    io.write_addr(addr);

    // set MREQ
    CpuFlagsIn flags;
    flags.mreq = true;
    flags.rd = true;
    flags.wr = true;
    io.write_flags(flags);
    waitk();

    // set RD
    flags.mreq = false;
    io.write_flags(flags);
    waitk();

    flags.rd = false;
    io.write_flags(flags);
    waitk();

    // read data
    uint8_t data = io.read_data();
    waitk();

    // return to normal
    io.set_high_impedance();
    return data;
}

void
Computer::write_ram(uint16_t addr, uint8_t data) const
{
    // set addr & data bus
    clear_bus();
    io.write_data(data);
    io.write_addr(addr);

    // set WE & MREQ
    CpuFlagsIn flags;
    flags.mreq = true;
    flags.wr = true;
    flags.rd = true;
    io.write_flags(flags);

    /*
    flags.mreq = false;
    io.write_flags(flags);

    flags.wr = false;
    io.write_flags(flags);

    // return to original
    flags.mreq = true;
    flags.wr = true;
    io.write_flags(flags);
    io.set_high_impedance();
    */
}

// vim:ts=4:sts=4:sw=4:expandtab
