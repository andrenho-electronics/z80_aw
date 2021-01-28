#include "diskview.hh"

#include "z80aw.hh"

bool DiskView::is_connected() const
{
    return z80aw::has_disk();
}

void DiskView::update()
{

}

void DiskView::go_to_block(uint32_t block)
{

}

DiskDataType DiskView::data_type(uint16_t pos) const
{
    return DiskDataType();
}

