#include "memoryview.hh"

#include "z80aw.hh"

void MemoryView::update()
{
    if (z80_state_.mode == Z80State::Stopped) {
        data_ = z80aw::read_block(((uint8_t) page_number_) * 0x100, 0x100);
        if (z80_state_.registers.has_value()) {
            uint16_t sp = z80_state_.registers.value().SP;
            stack_ = z80aw::read_block(sp, STACK_SIZE * 2);
        }
    }
}

void MemoryView::go_to_page(uint8_t page)
{
    page_number_ = page;
    update();
}
