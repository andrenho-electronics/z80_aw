#ifndef EMULATOR_SOURCE_HH
#define EMULATOR_SOURCE_HH


#include "memory.hh"

class Source : public Window {
public:
    std::string name() const override { return "Source Code"; }
};


#endif //EMULATOR_SOURCE_HH
