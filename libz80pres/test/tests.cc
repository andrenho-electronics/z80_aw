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
    
    // check code view
    auto lines = p.codeview().lines();
    ASSERT("Opens at the first file", p.codeview().file_selected().value_or("") == "project1.z80");
    ASSERT("4 lines in the first file", p.codeview().lines().size() == 4);
    
    ASSERT("First line", p.codeview().lines().at(0).code == "TEST = 0x1234");
    ASSERT("First line address", p.codeview().lines().at(0).address.has_value());
    ASSERT("First line bytes", p.codeview().lines().at(0).bytes.empty());
    
    ASSERT("Second line", p.codeview().lines().at(0).code == "\tnop");
    ASSERT("Second line address", p.codeview().lines().at(0).address.value_or(0xff) == 0);
    ASSERT("Second line bytes", p.codeview().lines().at(0).bytes == std::vector<uint8_t> { 0 });
    ASSERT("Second line PC", p.codeview().lines().at(0).is_pc);
    
    // TODO - file list (order)
    // TODO - go to another file
    // TODO - go to another symbol
    // TODO - step
}
