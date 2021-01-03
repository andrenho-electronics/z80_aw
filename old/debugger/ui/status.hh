#ifndef EMULATOR_STATUS_HH
#define EMULATOR_STATUS_HH

#include "window.hh"

class Status : public Window {
public:
    std::string name() const override { return "Status"; }

    void redraw() const override;
    void update() const override;

    void set_running() const override;
};

#endif
