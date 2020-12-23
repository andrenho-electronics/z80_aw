#ifndef EMULATOR_TERMINAL_HH
#define EMULATOR_TERMINAL_HH

#include "window.hh"

class Terminal : public Window {
public:
    std::string name() const override { return "Terminal"; }

};


#endif //EMULATOR_TERMINAL_HH
