#ifndef LIBZ80AW_MEMORYVIEW_HH
#define LIBZ80AW_MEMORYVIEW_HH

#include <vector>

#include "view.hh"

class MemoryView : public View {
public:
    static const size_t STACK_SIZE = 32;
    explicit MemoryView(Z80State const& z80_state) : View(z80_state) {}
    
    void update();
    void go_to_page(uint8_t page);
    
    uint8_t page_number() const { return page_number_; }
    std::vector<uint8_t> const& data() const { return data_; }
    std::vector<uint8_t> const& stack() const { return stack_; }

private:
    uint8_t page_number_ = 0;
    std::vector<uint8_t> data_;
    std::vector<uint8_t> stack_;
};

#endif
