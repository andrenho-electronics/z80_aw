#include "ui/ui.hh"
#include "config/config.hh"
#include "hardware/hardware.hh"
#include "hardware/emulatedhardware.hh"
#include "compiler/compiler.hh"

#include <iostream>

int main(int argc, char* argv[])
{
    Config config(argc, argv);
    if (config.hardware_type() == Emulated) {
        hardware = std::make_unique<EmulatedHardware>();
    } else if (config.hardware_type() == Real) {
        std::cerr << "Sorry, support for real hardware is not yet implemented.\n";
        return EXIT_FAILURE;
    } else {
        abort();
    }

    UI::init_curses();
    UI ui;

reload:
    try {
        Result r = compile_assembly_code(config.config_file());
    } catch (std::exception& e) {
        std::cerr << "Compilation error:\n\n" << e.what() << "\n";
        return EXIT_FAILURE;
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