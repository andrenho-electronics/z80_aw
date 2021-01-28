#ifndef LIBZ80AW_DISKVIEW_HH
#define LIBZ80AW_DISKVIEW_HH

#include <string>
#include <vector>

enum DataType { Unclassified, BootSector };

struct DiskDataType {
    DataType    data_type;
    std::string description;
};

class DiskView {
public:
    bool is_connected() const;
    
    void update();
    void go_to_block(uint32_t block);
    
    uint8_t block_number() const { return block_number_; }
    std::vector<uint8_t> const& data() const { return data_; }
    
    DiskDataType data_type(uint16_t pos) const;

private:
    uint8_t block_number_ = 0;
    std::vector<uint8_t> data_;
};

#endif
