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
    flags.mreq = false;
    flags.rd = true;
    flags.wr = true;
    io.write_flags(flags);
    waitk();

    // set RD
    flags.rd = false;
    io.write_flags(flags);

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
    waitk();

    // set WE & MREQ
    CpuFlagsIn flags;
    flags.mreq = false;
    flags.wr = false;
    flags.rd = true;
    io.write_flags(flags);
    waitk();
    
    // return to original
    io.set_high_impedance();
    waitk();
}

// vim:ts=4:sts=4:sw=4:expandtab
