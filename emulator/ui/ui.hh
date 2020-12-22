#ifndef EMULATOR_UI_HH
#define EMULATOR_UI_HH

#include "status.hh"

class UI {
public:
    UI();
    ~UI();

    bool active() const { return active_; }
    void execute();

private:
    void redraw() const;

    Status status;
    bool active_ = true;
};

#endif