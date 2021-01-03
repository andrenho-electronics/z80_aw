#ifndef EMULATOR_MEMORY_HH
#define EMULATOR_MEMORY_HH


#include "window.hh"

class Memory : public Window {
public:
    std::string name() const override { return "Memory"; }

    void redraw() const override;
    void update() const override;

    void change_page(int rel);
    void update_page(uint8_t page_);

    void set_running() const override;

private:
    uint8_t page = 0;
};


#endif //EMULATOR_MEMORY_HH
