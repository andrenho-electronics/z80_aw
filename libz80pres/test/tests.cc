#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "../z80pres.hh"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31mX\e[0m\n"); exit(1); }

int main()
{
    Z80Presentation p = Z80Presentation::initialize_with_emulator("../emulator");
    
    // compile invalid code
    try {
        p.compile_project_vasm("z80src/project_error.toml");
        ASSERT("Invalid source code.", false);
    } catch (std::exception& e) {
        // std::cerr << e.what() << "\n";
        ASSERT("Invalid source code.", true);
    }
    
    // compile valid code
    p.compile_project_vasm("z80src/project.toml");
    ASSERT("Valid source code.", true);
}
