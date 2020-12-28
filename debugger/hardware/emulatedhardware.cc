#include "emulatedhardware.hh"

EmulatedHardware::EmulatedHardware()
    : z80_({}), memory_()
{
    z80_.TrapBadOps = 1;
    z80_.User = this;
    ResetZ80(&z80_);
}

void EmulatedHardware::step()
{
    RunZ80(&z80_);
}

void EmulatedHardware::reset()
{
    z80_ = {};
    z80_.TrapBadOps = 1;
    z80_.User = this;
    ResetZ80(&z80_);
}

std::vector<uint8_t> EmulatedHardware::get_memory(uint16_t addr, uint16_t sz) const
{
    std::vector<uint8_t> r;
    std::copy(&memory_[addr], &memory_[addr + sz], std::back_inserter(r));
    return r;
}

void EmulatedHardware::upload()
{
    for (auto const& st: upload_staging_areas_) {
        uint16_t i = 0;
        for (uint8_t b: st.data)
            set_memory(st.addr + (i++), b);
    }
    set_memory(CHECKSUM_ADDR, upload_staging_checksum_ & 0xff);
    set_memory(CHECKSUM_ADDR + 1, upload_staging_checksum_ >> 8);
}
