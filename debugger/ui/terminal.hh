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
    void ask_keypress();

    void keypress(uint8_t key);

private:
    WINDOW* subwindow_;
    int     cursor_line_ = 0,
            cursor_col_ = 0;
};

#endif //EMULATOR_TERMINAL_HH
