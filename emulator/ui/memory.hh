#ifndef EMULATOR_MEMORY_HH
#define EMULATOR_MEMORY_HH


#include "window.hh"

class Memory : public Window {
public:
    std::string name() const override { return "Memory"; }

    void redraw() const override;
};


#endif //EMULATOR_MEMORY_HH
