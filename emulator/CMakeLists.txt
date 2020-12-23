cmake_minimum_required(VERSION 3.17)
project(emulator)

set(CMAKE_CXX_STANDARD 20)

include_directories(include include/pdcurses z80)
link_directories(lib)
link_libraries(pdcurses)

add_executable(emulator main.cc ui/ui.cc ui/ui.hh ui/status.cc ui/status.hh ui/window.cc ui/window.hh z80/Z80.c z80/z80_user.cc config/config.cc config/config.hh hardware/hardware.hh hardware/emulatedhardware.cc hardware/emulatedhardware.hh hardware/hardware.cc compiler/compiler.hh compiler/compiler.cc ui/colors.hh ui/memory.cc ui/memory.hh ui/source.cc ui/source.hh ui/terminal.cc ui/terminal.hh)