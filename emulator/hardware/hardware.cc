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
