#ifndef EMULATOR_WINDOW_HH
#define EMULATOR_WINDOW_HH

#include <string>

#include <curses.h>

class Window {
public:
    Window(int x, int y, int w=0, int h=0);
    ~Window();

    void redraw() const;

    virtual std::string name() const = 0;

protected:
    int x, y, w, h;
    WINDOW* window_;
};

#endif //EMULATOR_WINDOW_HH
