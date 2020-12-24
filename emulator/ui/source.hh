#ifndef EMULATOR_SOURCE_HH
#define EMULATOR_SOURCE_HH


#include "memory.hh"

class Source : public Window {
public:
    Source();
    ~Source();
    void resize(int line, int col, int lines, int cols) override;

    std::string name() const override { return "Source Code"; }

    void update() const override;

private:
    WINDOW* subwindow_;
    mutable size_t scroll_ = 0;
};

#endif //EMULATOR_SOURCE_HH
