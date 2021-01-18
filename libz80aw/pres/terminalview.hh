#ifndef LIBZ80AW_TERMINALVIEW_HH
#define LIBZ80AW_TERMINALVIEW_HH

#include <string>
#include <vector>

class TerminalView {
public:
    TerminalView(size_t lines, size_t columns);
    void add_char(uint8_t c);
    
    size_t cursor_x() const { return cursor_x_; }
    size_t cursor_y() const { return cursor_y_; }
    std::vector<std::string> const& text() const { return text_; }
    
    void reset();

private:
    size_t lines_, columns_;
    size_t cursor_x_ = 0, cursor_y_ = 0;
    std::vector<std::string> text_ {};
    
    void pull_screen_up();
};

#endif
