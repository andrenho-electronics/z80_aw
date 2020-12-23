#ifndef EMULATOR_UI_HH
#define EMULATOR_UI_HH

#include "status.hh"
#include "memory.hh"
#include "source.hh"
#include "terminal.hh"

class UI {
public:
    UI();
    ~UI();

    bool active() const { return active_; }
    void execute();

    static void init_curses();

private:
    void redraw();

    Status status;
    Memory memory;
    Source source;
    Terminal terminal;
    bool active_ = true;
};

#endif