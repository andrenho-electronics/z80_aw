#ifndef EMULATOR_WINDOW_HH
#define EMULATOR_WINDOW_HH

#include <string>

#include <curses.h>

class Window {
public:
    Window();
    ~Window();

    void resize(int line, int col, int lines=0, int cols=0);
    virtual void redraw() const;

    virtual std::string name() const = 0;

protected:
    WINDOW* window_;
    int line_ = 0, col_ = 0, lines_ = 0, cols_ = 0;
};

#endif //EMULATOR_WINDOW_HH
