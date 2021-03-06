#ifndef LIBZ80AW_DISKVIEW_HH
#define LIBZ80AW_DISKVIEW_HH

#include <string>
#include <vector>
#include "z80aw.hh"

enum DataType {
    Unclassified,
    BootstrapCode, BootSectorSignature,
    BytesPerSector, SectorsPerCluster, ReservedSectors, NumberOfFats, MaxNumberOfRootEntries, NumberOfSectors,
    MediaDescriptor, SectorsPerFat, SectorsPerTrack, NumberOfHeads, HiddenSectors,
    DriveNumber, VolumeSerialNumber, VolumeLabel, FileSystemType, FormattingOS,
};

struct DiskDataType {
    DataType    data_type;
    std::string description;
};

using SDCardStatus = z80aw::SDCardStatus;

class DiskView {
public:
    bool is_connected() const;
    
    void update();
    void go_to_block(uint32_t block);
    void go_to_next_block();
    void go_to_previous_block();
    
    uint32_t block_number() const { return block_number_; }
    std::vector<uint8_t> const& data() const { return data_; }
    
    DiskDataType data_type(uint16_t pos) const;
    SDCardStatus last_status() const { return last_status_; }

private:
    uint32_t block_number_ = 0;
    std::vector<uint8_t> data_;
    SDCardStatus last_status_ { SD_NOT_INITIALIZED, 0xff };
};

#endif
