#include "ui/ui.hh"
#include "config/config.hh"
#include "hardware/hardware.hh"
#include "hardware/emulatedhardware.hh"
#include "compiler/compiler.hh"
#include "hardware/realhardware.hh"

#include <iostream>

int main(int argc, char* argv[])
{
    Config config(argc, argv);
    if (config.hardware_type() == Emulated) {
        hardware = std::make_unique<EmulatedHardware>();
    } else if (config.hardware_type() == Real) {
        hardware = std::make_unique<RealHardware>(config.serial_port());
    } else {
        abort();
    }

    UI::init_curses();
    UI ui;

reload:
    try {
        Result r = compile_assembly_code(config);
    } catch (std::exception& e) {
        UI::display_error(std::string("Compilation error:\n\n") + e.what());
        goto reload;
    }

    ui.initialize();

    while (ui.active()) {
        ui.update();
        if (ui.execute()) {
            compiled_code.reset();
            hardware->reset();
            goto reload;
        }
    }
}