#ifndef EMULATOR_TERMINAL_HH
#define EMULATOR_TERMINAL_HH

#include "window.hh"

class Terminal : public Window {
public:
    Terminal();
    ~Terminal();
    void resize(int line, int col, int lines, int cols) override;

    void update() const override;

    std::string name() const override { return "Terminal"; }

    void print_char(uint8_t ch);
    void keypress();

    uint8_t last_keypress() const { return last_keypress_; }

    bool keyboard_interrupt() const { return keyboard_interrupt_; }
    void clear_keyboard_interrupt() { keyboard_interrupt_ = false; }

private:
    WINDOW* subwindow_;
    int     cursor_line_ = 0,
            cursor_col_ = 0;
    bool    keyboard_interrupt_ = false;
    uint8_t last_keypress_ = 0;
};

extern Terminal* global_terminal;

#endif //EMULATOR_TERMINAL_HH
