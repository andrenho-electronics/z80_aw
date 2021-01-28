#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include <chrono>
#include <iostream>
#include <thread>
using namespace std::chrono_literals;

#include "../pres/z80pres.hh"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31mX\e[0m\n"); exit(1); } \
    if (opt.wait_after_each_assert) getc(stdin);

struct Options {
    bool  log_to_stdout = false;
    bool  wait_after_each_assert = false;
    char* serial_port = nullptr;

    Options(int argc, char* argv[]) {
        int opt;
        while ((opt = getopt(argc, argv, "lwp:h")) != -1) {
            switch (opt) {
                case 'l':
                    log_to_stdout = true;
                    break;
                case 'w':
                    wait_after_each_assert = true;
                    break;
                case 'h':
                    print_help(argv[0]);
                    exit(EXIT_SUCCESS);
                case 'p':
                    serial_port = optarg;
                    break;
                default:
                    print_help(argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
    }

private:
    void print_help(const char* program) {
        std::cout << "Usage: " << program << " [-lw] [-p SERIAL_PORT]\n";
        std::cout << "  -l    Log bytes to stdout\n";
        std::cout << "  -w    Wait after each assert\n";
        std::cout << "  -p    Connect to serial port instead of starting the emulator\n";
    }
};


int main(int argc, char* argv[])
{
    Options opt(argc, argv);

    Z80Presentation p(opt.serial_port ? opt.serial_port : ".", !opt.serial_port);
    p.set_logging_to_stdout(opt.log_to_stdout);
    
    //
    // COMPILATION
    //
    
    // compile invalid code
    try {
        p.compile_project(CompilerType::Vasm, "z80src/project_error.toml");
        ASSERT("Invalid source code.", false);
    } catch (std::exception& e) {
        // std::cerr << e.what() << "\n";
        ASSERT("Invalid source code.", true);
    }
    
    // compile valid code
    p.compile_project(CompilerType::Vasm, "z80src/project.toml");
    p.upload_compiled();
    ASSERT("Valid source code.", true);
    
    //
    // CODEVIEW
    //
    
    // check code view
    auto lines = p.codeview().lines();
    std::cout << "File currently selected: " << p.codeview().file_selected().value_or("no file selected") << "\n";
    ASSERT("Opens at the first file", p.codeview().file_selected().value_or("") == "project1.z80");
    ASSERT("8 lines in the first file", p.codeview().lines().size() == 8);
    
    ASSERT("First line", p.codeview().lines().at(0).code == "TEST = 0x1234");
    ASSERT("First line address", !p.codeview().lines().at(0).address.has_value());
    ASSERT("First line bytes", p.codeview().lines().at(0).bytes.empty());
    
    ASSERT("Third line", p.codeview().lines().at(2).code == "        nop ; PC = 0, 00");
    ASSERT("Third line address", p.codeview().lines().at(2).address.value_or(0xff) == 0);
    ASSERT("Third line PC", p.codeview().lines().at(2).is_pc);
    ASSERT("Third line bytes", p.codeview().lines().at(2).bytes == std::vector<uint8_t> { 0 });
    
    // select another file
    p.codeview().set_file("project2.z80");
    ASSERT("Opens at the new file", p.codeview().file_selected().value_or("") == "project2.z80");
    ASSERT("First line", p.codeview().lines().at(0).code == "        org     0x10");
    ASSERT("Third line address", p.codeview().lines().at(2).address.value_or(-1) == 0x10);
    ASSERT("Third line address", p.codeview().lines().at(2).bytes == std::vector<uint8_t> { 0xcf });
    
    // file list (order)
    ASSERT("File list", (p.file_list() == std::vector<std::string> {
        "project1.z80", "project_include.z80", "project2.z80"
    }));
    
    // symbols (order)
    ASSERT("Symbol list", (p.symbol_list() == std::vector<Symbol> {
            { "main", "project_include.z80", 1, 0x1 },   // TODO - this is not exacly correct
            { "project2", "project2.z80", 3, 0x10 },
    }));
    
    // go to another symbol
    ASSERT("Symbol line", p.codeview().goto_symbol("project2") == 3);
    ASSERT("Select symbol", p.codeview().file_selected().value_or("") == "project2.z80");
    
    // step
    p.reset();
    p.step();
    std::cout << "File currently selected: " << p.codeview().file_selected().value_or("no file selected") << "\n";
    ASSERT("Check open file", p.codeview().file_selected().value_or("") == "project_include.z80");
    ASSERT("PC = first line", p.codeview().lines().at(0).is_pc);
    ASSERT("First line address", p.codeview().lines().at(0).address.value_or(-1) == 0x1);
    
    // add/remove breakpoints
    p.codeview().add_breakpoint(2);
    ASSERT("Check inserted breakpoint", p.codeview().lines().at(1).is_breakpoint);
    p.codeview().remove_breakpoint(2);
    ASSERT("Check remove breakpoint", !p.codeview().lines().at(1).is_breakpoint);
    p.remove_all_breakpoints();
    
    // continue
    p.compile_project(CompilerType::VasmCode, R"(
        ld  a, 0x45       ; PC = 0, line = 2
        ld  (0xa001), a   ; PC = 2, line = 3
    temp:
        jp  temp          ; PC = 5, line = 5
    )");
    p.reset();
    p.continue_();
    std::this_thread::sleep_for(10ms);
    p.stop();
    ASSERT("Stopped at the correct location", p.pc() == 0x5);
    
    // get register information (register fetch disabled)
    p.step();
    ASSERT("Register fetch disabled", !p.registers().has_value());
    
    // get register information (register fetch enabled)
    if (opt.serial_port == NULL) {  // only test this with emulator
        p.set_register_fetch_mode(RegisterFetchMode::Emulator);
        p.step();
        ASSERT("Register fetch enabled", (p.registers().value().AF >> 8) == 0x45);
        p.set_register_fetch_mode(RegisterFetchMode::Disabled);
    }
    
    // test breakpoints
    p.codeview().add_breakpoint(5);
    p.reset();
    p.continue_();
    std::this_thread::sleep_for(10ms);
    p.check_events();
    ASSERT("Test if has stopped at breakpoint (mode == STOPPED)", p.mode() == Z80State::Stopped);
    ASSERT("Stopped at the correct location", p.pc() == 0x5);
    p.remove_all_breakpoints();
    
    // check registers
    if (opt.serial_port == NULL) {  // only test this with emulator
        ASSERT("Check registers set after stop at breakpoint", (p.registers().value().AF >> 8) == 0x45);
    }
    
    // next
    p.compile_project(CompilerType::VasmCode, R"(
        nop         ; PC = 0
        call sr     ; PC = 1
    st: jp st       ; PC = 4
    sr: nop         ; PC = 7
        ret         ; PC = 8
    )");
    p.reset();
    p.next();
    std::this_thread::sleep_for(10ms);
    p.check_events();
    ASSERT("Next: regular opcode", p.pc() == 0x1);
    p.next();
    std::this_thread::sleep_for(10ms);
    p.check_events();
    ASSERT("Next: subroutine opcode", p.pc() == 0x4);
    
    //
    // MEMORY VIEW
    //
    p.compile_project(CompilerType::VasmCode, R"(
        ld   sp, 0xfffe
        ld   a, 0xbd
        ld   (0x9301), a
        ld   hl, 0x1234
        push hl
    )");
    p.reset();
    if (opt.serial_port == NULL) {  // only test this with emulator
        p.set_register_fetch_mode(RegisterFetchMode::Emulator);
    }
    p.step(); p.step(); p.step(); p.step(); p.step();
    ASSERT("Data is set correctly (1)", p.memoryview().data().at(0) == 0x31);  // ld sp, **
    ASSERT("Data is set correctly (2)", p.memoryview().data().at(3) == 0x3e);  // ld a, *
    if (opt.serial_port == NULL) {  // only test this with emulator
        ASSERT("Check stack (1)", p.memoryview().stack().at(0) == 0x34);
        ASSERT("Check stack (2)", p.memoryview().stack().at(1) == 0x12);
    }
    
    // go to page
    p.memoryview().go_to_page(0x93);
    ASSERT("Data is set correctly", p.memoryview().data().at(1) == 0xbd);
    if (opt.serial_port == NULL) {  // only test this with emulator
        ASSERT("Check stack (1) - no change", p.memoryview().stack().at(0) == 0x34);
        ASSERT("Check stack (2) - no change", p.memoryview().stack().at(1) == 0x12);
    }
    
    //
    // TERMINAL
    //
    p.compile_project(CompilerType::VasmCode, R"(
        ld  a, 'x'
        out (0x0), a
    )");
    p.reset();
    p.step();
    p.step();
    ASSERT("Check that character was sent to terminal", p.terminalview().text().at(0).at(0) == 'x');
    ASSERT("Check that cursor moved forward", p.terminalview().cursor_x() == 1);
    ASSERT("Check that cursor stayed on the same line", p.terminalview().cursor_y() == 0);
    
    // terminal run
    p.compile_project(CompilerType::VasmCode, R"(
            ld a, 'A'
            out (0), a      ; device 0x0 = video
            ld a, 'W'
            out (0), a
         x: jp x
    )");
    p.reset();
    p.continue_();
    auto now = std::chrono::system_clock::now();
    while (std::chrono::system_clock::now() < (now + 20ms))
        p.check_events();
    p.check_events();
    p.check_events();
    ASSERT("[continue] First character was printed", p.terminalview().text().at(0).at(0) == 'A');
    ASSERT("[continue] Second character was printed", p.terminalview().text().at(0).at(1) == 'W');
    ASSERT("[continue] Check that cursor moved forward", p.terminalview().cursor_x() == 2);
    p.stop();
    
    //
    // SD CARD
    //
    
    p.compile_project(CompilerType::VasmDisk, "z80src/sdcard/sdcard.toml");
    p.upload_compiled();
    
    p.diskview().update();
    ASSERT("Check that disk data was loaded", p.diskview().data().at(510) == 0x55);
    ASSERT("Check that the disk data is verified correctly", p.diskview().data_type(480).data_type == BootSector);
    p.diskview().go_to_block(1);
    ASSERT("Check that disk data was loaded after changing blocks", p.diskview().data().at(510) != 0x55);
}
