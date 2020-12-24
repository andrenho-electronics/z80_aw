#ifndef EMULATOR_SOURCE_HH
#define EMULATOR_SOURCE_HH


#include "memory.hh"
#include "../compiler/compiler.hh"

class Source : public Window {
public:
    Source();
    ~Source();
    void resize(int line, int col, int lines, int cols) override;

    std::string name() const override { return "Source Code"; }

    void update() const override;
    void pc_updated();

private:
    WINDOW* subwindow_;
    size_t scroll_ = 0;
    int cursor_line_ = 0;
    SourceLocation source_location_ {};
};

#endif //EMULATOR_SOURCE_HH
