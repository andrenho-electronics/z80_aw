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

void Hardware::add_to_upload_staging(std::vector<uint8_t> const& data, uint16_t addr)
{
    upload_staging_areas_.push_back({ data, addr });
    
    // calculate checksum
    uint16_t checksum1 = 0, checksum2 = 0;
    for (auto const& st: upload_staging_areas_) {
        for (uint8_t b: st.data) {
            checksum1 = (checksum1 + b) % 255;
            checksum2 = (checksum2 + checksum1) % 255;
        }
    }
    upload_staging_checksum_ = checksum1 | (checksum2 << 8);
}

bool Hardware::matching_upload_checksum() const
{
    return (get_memory(CHECKSUM_ADDR) == (upload_staging_checksum_ & 0xff))
        && (get_memory(CHECKSUM_ADDR + 1) == (upload_staging_checksum_ >> 8));
}
