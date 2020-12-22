cmake_minimum_required(VERSION 3.17)
project(emulator)

set(CMAKE_CXX_STANDARD 20)

include_directories(include include/pdcurses)
link_directories(lib)
link_libraries(pdcurses)

add_executable(emulator main.cc)