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
    void update();
    static void draw_status_bar() ;

    static long ask(std::string const& question) ;

    static void init_curses();

private:
    static uint8_t translate_char(int ch);

    void redraw();
    void run();
    void step();

    Status status;
    Memory memory;
    Source source;
    Terminal terminal;
    bool active_ = true;
};

#endif