#ifndef EMULATOR_STATUS_HH
#define EMULATOR_STATUS_HH

#include "window.hh"

class Status : public Window {
public:
    Status() : Window(COLS - 25, 0) {}

    std::string name() const override { return "Status"; }
};

#endif
