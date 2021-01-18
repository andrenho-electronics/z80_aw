#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include "options.hh"

Options::Options(int argc, char** argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "hd")) != -1) {
        switch (opt) {
            case 'h':
                show_help(argv[0]);
                exit(EXIT_SUCCESS);
            case 'd':
                show_demo_window_ = true;
                break;
            default:
                show_help(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

void Options::show_help(const char* program)
{
    std::cout << "A debugger for the Z80AW machine.\n";
    std::cout << "Usage: " << program << " [-d]\n";
    std::cout << "Options:\n";
    std::cout << "   -d      Show imgui demo window\n";
}
