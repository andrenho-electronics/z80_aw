#include "hardware.hh"

#include <memory>

std::unique_ptr<Hardware> hardware;

void Hardware::add_breakpoint(uint16_t addr)
{
    breakpoints_.insert(addr);
}

void Hardware::remove_breakpoint(uint16_t addr)
{
    breakpoints_.erase(addr);
}

bool Hardware::is_breakpoint(uint16_t addr) const
{
    return breakpoints_.find(addr) != breakpoints_.end();
}

bool Hardware::next_is_subroutine() const
{
    uint8_t next = hardware->get_memory(hardware->PC());
    switch (next) {
        case 0xcd:  // call
        case 0xdc:
        case 0xfc:
        case 0xd4:
        case 0xc4:
        case 0xf4:
        case 0xec:
        case 0xe4:
        case 0xcc:
        case 0xc7:  // rst
        case 0xcf:
        case 0xd7:
        case 0xdf:
        case 0xe7:
        case 0xef:
        case 0xf7:
        case 0xff:
            return true;
        default:
            return false;
    }
}

void Hardware::add_breakpoint_next()
{
    uint8_t next = hardware->get_memory(hardware->PC());
    switch (next) {
        case 0xcd:  // call
        case 0xdc:
        case 0xfc:
        case 0xd4:
        case 0xc4:
        case 0xf4:
        case 0xec:
        case 0xe4:
        case 0xcc:
            add_breakpoint(hardware->PC() + 3);
            break;
        case 0xc7:  // rst
        case 0xcf:
        case 0xd7:
        case 0xdf:
        case 0xe7:
        case 0xef:
        case 0xf7:
        case 0xff:
            add_breakpoint(hardware->PC() + 1);
            break;
    }
}
