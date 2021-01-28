#include "diskview.hh"

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
    if (block_number_ == 0) {
        if (pos < 3 || pos > 0x3e)
            return { BootSector, "Bootstrap code" };
    }
    return { Unclassified, "Unclassified data" };
}

