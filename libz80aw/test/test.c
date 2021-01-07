#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../z80aw.h"
#include "../compiler.h"
#include "protocol.h"
#include "../comm.h"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31m❌\e[0m\n"); exit(1); }

#define COMPILE(code) {                                                                  \
    char errbuf_[4096] = "";                                                             \
    int resp_ = z80aw_simple_compilation(code, errbuf_, sizeof errbuf_); \
    if (resp_ != 0) {                                                                    \
        printf("Compilation error: %s\n", errbuf_);                                      \
        exit(1);                                                                         \
    }                                                                                    \
}

typedef enum { EMULATOR, REALHARDWARE } HardwareType;
typedef struct {
    HardwareType hardware_type;
    const char*  serial_port;
    bool         log_to_stdout;
} Config;

Config initialize(int argc, char* argv[])
{
    Config config = { .hardware_type = EMULATOR };
    
    int opt;
    while ((opt = getopt(argc, argv, "hr:l")) != -1) {
        switch (opt) {
            case 'l':
                config.log_to_stdout = true;
                break;
            case 'r':
                config.hardware_type = REALHARDWARE;
                config.serial_port = optarg;
                break;
            default:
                printf("Usage: %s [-r PORT]\n", argv[0]);
                printf("     -r      Run on real hardware, where PORT is the serial port (ex. /dev/ttyUSB0)\n");
                printf("     -l      Log bytes to stdout\n");
                exit(EXIT_FAILURE);
        }
    }
    
    return config;
}

int main(int argc, char* argv[])
{
    char serial_port[128];
    Config config = initialize(argc, argv);
    
    if (config.hardware_type == EMULATOR) {
        if (z80aw_initialize_emulator("../emulator", serial_port, sizeof serial_port) != 0) {
            fprintf(stderr, "Error initializing emulator: %s", z80aw_last_error());
            exit(1);
        }
    } else {
        strcpy(serial_port, config.serial_port);
    }
    
    printf("Serial port: %s\n", serial_port);
    
    Z80AW_Config cfg = {
            .serial_port         = serial_port,
            .log_to_stdout       = config.log_to_stdout,
            .assert_empty_buffer = true,
    };
    z80aw_init(&cfg);
    
    uint8_t block[MAX_BLOCK_SIZE], rblock[MAX_BLOCK_SIZE];
    
    //
    // generic commands
    //
    
    ASSERT("Invalid command", zsend_expect(Z_ACK_REQUEST, 0) == -1);
    ASSERT("Error message", strcmp(z80aw_last_error(), "No error.") != 0);
    
    //
    // empty buffer
    //
    
    zsend_noreply(Z_ACK_REQUEST);
    if (config.log_to_stdout) printf("\n");
    ASSERT("Empty buffer (not empty)", !z_empty_buffer());
    zrecv();
    ASSERT("Empty buffer (empty)", z_empty_buffer());
    
    //
    // controller
    //
    
    ASSERT("Controller info - free memory", z80aw_controller_info().free_memory > 10);
    
    //
    // memory
    //
    
    uint8_t chk[] = { 0xfa, 0x80, 0x0, 0x79, 0xab };
    ASSERT("Checksum", z80aw_checksum(sizeof chk, chk) == 0x87a0);
    
    ASSERT("Write byte", z80aw_write_byte(0x8, 0xfa) == 0);
    ASSERT("Read byte", z80aw_read_byte(0x8) == 0xfa);
    
    for (size_t i = 0; i < MAX_BLOCK_SIZE; ++i)
        block[i] = (i + 1) & 0xff;
    ASSERT("Write block", z80aw_write_block(0x100, MAX_BLOCK_SIZE, block) == 0);
    ASSERT("Read block", z80aw_read_block(0x100, MAX_BLOCK_SIZE, rblock) == 0);
    ASSERT("Compare blocks", memcmp(block, rblock, MAX_BLOCK_SIZE) == 0);
    
    //
    // compiler
    //
    
    DebugInformation* di = compile_vasm("z80src/project.toml");
    ASSERT("DebugInformation is not null", di);
    ASSERT("Compiler output is successful", debug_output(di, NULL, 0));
    if (config.log_to_stdout) {
        printf("Compiler output:\n\e[0;33m");
        debug_print(di);
        printf("\e[0m\n");
    }
    
    DebugInformation* di_error = compile_vasm("z80src/project_error.toml");
    ASSERT("DebugInformation is not null", di_error);
    {
        char error_msg[4096];
        ASSERT("Compiler output is not successful", !debug_output(di_error, error_msg, sizeof error_msg));
        if (config.log_to_stdout) {
            printf("Compiler error output:\n");
            printf("\e[0;33m%s\e[0m\n\n", error_msg);
        }
    }
    debug_free(di_error);
    
    // upload compiled code
    ASSERT("Check checksum (not uploaded)", !z80aw_is_uploaded(di));
    ASSERT("Upload successful", z80aw_upload_compiled(di) == 0);
    z80aw_read_block(0x0, debug_binary(di, 0)->sz, rblock);
    ASSERT("Test block 1 upload", memcmp(rblock, debug_binary(di, 0)->data, debug_binary(di, 0)->sz) == 0);
    ASSERT("Test block 2 upload", z80aw_read_byte(0x10) == 0xcf);
    ASSERT("Check checksum (uploaded)", z80aw_is_uploaded(di));
    
    // simple compilation
    char errbuf[4096] = "";
    int resp = z80aw_simple_compilation(" jp 0xc3c3\n rst 0x8", errbuf, sizeof errbuf);
    ASSERT("Simple compilation", resp == 0);
    {
        uint8_t expected[] = { 0xc3, 0xc3, 0xc3, 0xcf };
        uint8_t found[4];
        z80aw_read_block(0x0, sizeof found, found);
        ASSERT("Compare blocks", memcmp(expected, found, sizeof found) == 0);
    }
    
    resp = z80aw_simple_compilation("error error", errbuf, sizeof errbuf);
    if (config.log_to_stdout)
        printf("Compiler error output:\n\e[0;33m%s\e[0m\n\n", errbuf);
    ASSERT("Simple compilation with error", resp != 0);
    ASSERT("Compiler error message", strlen(errbuf) > 5);
    
    //
    // CPU operations
    //
    
    ASSERT("CPU reset", z80aw_cpu_reset() == 0);
    Z80AW_Registers r;
    ASSERT("Retrieve registers", z80aw_cpu_registers(&r) == 0);
    ASSERT("PC == 0", r.PC == 0);
   
    // single step
    uint8_t jp[] = { 0xc3, 0xc3, 0xc3 };
    z80aw_write_block(0, sizeof jp, jp);
    ASSERT("Step (jp 0xc3c3)",z80aw_cpu_step(NULL) == 0);
    z80aw_cpu_registers(&r);
    ASSERT("PC == 0xC3C3", r.PC == 0xc3c3);
    
    // compile and execute step
    COMPILE(" ld a, 0x42");
    ASSERT("Step (ld a, 0x42)", z80aw_cpu_step(NULL) == 0);
    z80aw_cpu_registers(&r);
    ASSERT("A == 0x42", (r.AF >> 8) == 0x42);
    
    // char on the screen
    COMPILE(" ld a, 'H'\n out (0), a\n nop");   // device 0x0 = video
    uint8_t c;
    z80aw_cpu_step(NULL);
    z80aw_cpu_step(&c);
    ASSERT("Char printed = 'H'", c == 'H');
    z80aw_cpu_step(&c);
    ASSERT("Print char is cleared", c == 0);
    
    // receive keypress
    COMPILE(" in a, (0x1)");   // device 0x1 = keyboard
    z80aw_keypress('6');
    z80aw_cpu_step(NULL);
    z80aw_cpu_registers(&r);
    ASSERT("Receive keypress", (r.AF >> 8) == '6');
    
    // keypress interrupt
    COMPILE(" jp main\n"
            " org 0x8 \n"
            " halt    \n"
            "main:    \n"
            " im 0    \n"
            " ei      \n"
            "cc: jp cc");
    for (size_t i = 0; i < 16; ++i)
        z80aw_cpu_step(NULL);
    z80aw_keypress('k');
    for (size_t i = 0; i < 16; ++i)
        z80aw_cpu_step(NULL);
    z80aw_cpu_registers(&r);
    ASSERT("Keyboard interrupt was received", r.HALT);
    
    //
    // breakpoint setting
    //
    
    uint16_t bkps[16];
    ASSERT("Add breakpoint", z80aw_add_breakpoint(0xf00) == 0);
    ASSERT("Querying breakpoints", z80aw_query_breakpoints(bkps, 16) == 1);
    ASSERT("Checking brekpoints", bkps[0] == 0xf00);
    ASSERT("Add same breakpoint", z80aw_add_breakpoint(0xf00) == 0);
    ASSERT("Check that we still have the one breakpoint", z80aw_query_breakpoints(bkps, 16) == 1);
    ASSERT("Add another breakpoint", z80aw_add_breakpoint(0x123) == 0);
    ASSERT("Check that we have two breakpoints", z80aw_query_breakpoints(NULL, 0) == 2);
    ASSERT("Remove one breakpoint", z80aw_remove_breakpoint(0xf00) == 0);
    ASSERT("Check that we have one breakpoint", z80aw_query_breakpoints(bkps, 16) == 1);
    ASSERT("Check that is the correct breakpoint", bkps[0] == 0x123);
    ASSERT("Remove all breakpoints", z80aw_remove_all_breakpoints() == 0);
    ASSERT("Check that we now have no breakpoints", z80aw_query_breakpoints(NULL, 0) == 0);
    
    //
    // continue execution
    //
    
    ASSERT("No last event", z80aw_last_event().type == Z80AW_NO_EVENT);
    
    // breakpoint hit
    COMPILE(" s: nop\n nop\n nop\n nop\n jp s");
    z80aw_add_breakpoint(0x3);
    ASSERT("Continue execution", z80aw_cpu_continue() == 0);
    while (z80aw_last_event().type != Z80AW_BREAKPOINT);
    z80aw_cpu_registers(&r);
    ASSERT("Stop at breakpoint", r.PC == 0x3);
    z80aw_remove_all_breakpoints();
    
    // keypress
    COMPILE(" jp main\n"
            " org 0x8 \n"
            " halt    \n"
            "main:    \n"
            " im 0    \n"
            " ei      \n"
            "cc: jp cc");
    z80aw_add_breakpoint(0x8);
    z80aw_cpu_continue();
    usleep(10000);
    z80aw_keypress('k');
    while (z80aw_last_event().type != Z80AW_BREAKPOINT);
    z80aw_cpu_registers(&r);
    ASSERT("Key was pressed during continue", r.HALT);
    
    // stop
    z80aw_remove_all_breakpoints();
    z80aw_cpu_reset();
    z80aw_cpu_continue();
    usleep(10000);
    z80aw_cpu_stop();
    z80aw_cpu_registers(&r);
    ASSERT("Stop stopped at the correct moment", r.PC == 0xc);
    
    z80aw_cpu_continue();
    z80aw_keypress('k');
    usleep(10000);
    z80aw_cpu_stop();
    z80aw_cpu_registers(&r);
    ASSERT("Stop stopped at the correct moment (after interrupt)", r.HALT);
    
    // print
    COMPILE(" ld a, 'A'\n"
            " out (0), a\n"      // device 0x0 = video
            " ld a, 'W' \n"
            " out (0), a\n"
            "x: jp x    \n");
    z80aw_cpu_continue();
    Z80AW_Event e = { .type = Z80AW_NO_EVENT };
    do {
        e = z80aw_last_event();
        if (e.type == Z80AW_PRINT_CHAR) {
            ASSERT("Check that character 'A' was printed", e.data == 'A');
        }
    } while (e.type != Z80AW_PRINT_CHAR);
    do {
        e = z80aw_last_event();
        if (e.type == Z80AW_PRINT_CHAR) {
            ASSERT("Check that character 'W' was printed", e.data == 'W');
        }
    } while (e.type != Z80AW_PRINT_CHAR);
    z80aw_cpu_stop();
    
    //
    // run cycles
    //
   
    z80aw_cpu_reset();
    Z80AW_Status s = z80aw_cpu_status();
    ASSERT("Check CPU status (after reset)", s.cycle == 0);
    ASSERT("CPU cycle", z80aw_cpu_cycle() == 0);
    s = z80aw_cpu_status();
    ASSERT("Check CPU status (after cycle)", s.cycle == 1);
    
    
    //
    // finalize
    //
    ASSERT("Finalizing emulator", zsend_expect(Z_EXIT_EMULATOR, Z_OK) == 0);
    
    debug_free(di);
    z80aw_close();
}