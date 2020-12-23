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


    CompiledCode cc;
    try {
        auto const [output, cc_] = compile_assembly_code(config.config_file());
        cc = cc_;
    } catch (std::exception& e) {
        std::cerr << "Compilation error:\n\n" << e.what() << "\n";
        return EXIT_FAILURE;
    }

    UI::init_curses();

    UI ui;
    while (ui.active()) {
        ui.update();
        ui.execute();
    }
}