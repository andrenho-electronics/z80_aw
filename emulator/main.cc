#include "ui/ui.hh"
#include "config/config.hh"
#include "hardware/hardware.hh"
#include "hardware/emulatedhardware.hh"
#include "compiler/compiler.hh"

#include <iostream>
#include <curses.h>

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

    auto r = compile_assembly_code(config.config_file());
    if (!r.ok) {
        std::cerr << "Compilation error:\n\n" << r.messages;
        return EXIT_FAILURE;
    }

    /*
    initscr();
    resize_term(40, 130);
    noecho();
    keypad(stdscr, true);
    refresh();

    UI ui;

    while (ui.active())
        ui.execute();
    */
}