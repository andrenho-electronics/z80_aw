cmake_minimum_required(VERSION 3.17)
project(emulator)

set(CMAKE_CXX_STANDARD 20)

include_directories(include include/pdcurses z80)
link_directories(lib)
link_libraries(pdcurses)

add_executable(emulator main.cc ui/ui.cc ui/ui.hh ui/status.cc ui/status.hh ui/window.cc ui/window.hh z80/Z80.c z80/z80_user.c config/config.cc config/config.hh)