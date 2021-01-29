#include "diskview.hh"

#include <cstring>
#include <sstream>

#include "z80aw.hh"

bool DiskView::is_connected() const
{
    return z80aw::has_disk();
}

void DiskView::update()
{
    data_ = z80aw::read_disk_block(block_number_);
}

void DiskView::go_to_block(uint32_t block)
{
    block_number_ = block;
    update();
}

DiskDataType DiskView::data_type(uint16_t pos) const
{
    if (data_.empty())
        return { Unclassified, "Unclassified data" };
    
    if (block_number_ == 0) {
        if (pos < 3 || (pos > 0x3e && pos < 0x1fe))
            return { BootstrapCode, "Bootstrap code" };
        else if (pos >= 0xb && pos < 0xd)
            return { BytesPerSector, std::to_string(data_.at(0xb) | (data_.at(0xc) << 8)) + " bytes per sector" };
        else if (pos == 0xd)
            return { SectorsPerCluster, std::to_string(data_.at(0xd)) + " sectors per cluster" };
        else if (pos >= 0xe && pos < 0x10)
            return { ReservedSectors, std::to_string(data_.at(0xb) | (data_.at(0xc) << 8)) + " reserved sectors" };
        else if (pos == 0x10)
            return { NumberOfFats, std::to_string(data_.at(0x10)) + " FAT copies" };
        else if (pos >= 0x11 && pos < 0x13)
            return { MaxNumberOfRootEntries, std::to_string(data_.at(0x11) | (data_.at(0x12) << 8)) + " possible root entries" };
        else if (pos >= 0x13 && pos < 0x15)
            return { NumberOfSectors, std::to_string(data_.at(0x13) | (data_.at(0x14) << 8)) + " sectors (if size < 32 MB)" };
        else if (pos == 0x15) {
            std::stringstream ss;
            ss << std::hex << "Media type: 0x" << data_.at(0x10);
            return { MediaDescriptor, ss.str() };
        } else if (pos >= 0x16 && pos < 0x18)
            return { SectorsPerFat, std::to_string(data_.at(0x16) | (data_.at(0x17) << 8)) + " sectors per FAT" };
        else if (pos >= 0x18 && pos < 0x1a)
            return { SectorsPerTrack, std::to_string(data_.at(0x18) | (data_.at(0x19) << 8)) + " sectors per track" };
        else if (pos >= 0x1a && pos < 0x1c)
            return { NumberOfHeads, std::to_string(data_.at(0x1a) | (data_.at(0x1b) << 8)) + " heads" };
        else if (pos >= 0x1c && pos < 0x20)
            return { HiddenSectors, std::to_string(data_.at(0x1c) | (data_.at(0x1d) << 8) | (data_.at(0x1e) << 16) | (data_.at(0x1f) << 24)) + " hidden sectors" };
        else if (pos >= 0x20 && pos < 0x24)
            return { NumberOfSectors, std::to_string(data_.at(0x20) | (data_.at(0x21) << 8) | (data_.at(0x22) << 16) | (data_.at(0x23) << 24)) + " sectors (if size > 32 MB)" };
        else if (pos == 0x24)
            return { DriveNumber, "Drive number " + std::to_string(data_.at(0x10)) };
        else if (pos >= 0x27 && pos < 0x2b) {
            std::stringstream ss;
            ss << std::hex << "Media type: 0x" << (data_.at(0x27) | (data_.at(0x28) << 8) | (data_.at(0x29) << 16) | (data_.at(0x2a) << 24));
            return { VolumeSerialNumber, ss.str() };
        } else if (pos >= 0x2b && pos < 0x36) {
            char buf[12] = { 0 };
            strncpy(buf, reinterpret_cast<char const*>(&data_[0x2b]), 11);
            return { VolumeLabel, buf };
        } else if (pos >= 0x36 && pos < 0x3e) {
            char buf[9] = { 0 };
            strncpy(buf, reinterpret_cast<char const*>(&data_[0x36]), 8);
            return { FileSystemType, buf };
        } else if (pos >= 0x1fe) {
            return { BootSectorSignature, "Boot sector signature" };
        }
    }
    return { Unclassified, "Unclassified data" };
}

