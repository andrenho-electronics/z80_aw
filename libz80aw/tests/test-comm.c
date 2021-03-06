#include <time.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../comm/z80aw.h"
#include "../comm/compiler.h"
#include "../comm/comm.h"
#include "../comm/logging.h"
#include "protocol.h"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31mX\e[0m\n"); exit(1); }

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

static void dump_memory(uint16_t addr, uint16_t sz)
{
    (void) dump_memory;
    uint8_t* data = (uint8_t*) malloc(sz);
    z80aw_read_block(addr, sz, data);
    printf("\n");
    for (uint16_t a = 0; a < sz; a += 0x10) {
        printf("%04X : ", addr + a);
        for (uint16_t i = 0; i < 0x10; ++i)
            printf("%02X ", data[a + i]);
        printf("\n");
    }
    free(data);
}

Config initialize(int argc, char* argv[])
{
    Config config = { 
        .hardware_type = EMULATOR, 
        .serial_port   = NULL, 
        .log_to_stdout = false,
    };
    
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

static void error_cb(const char* error, void* data)
{
    (void) data;
    fprintf(stderr, "ERROR DETECTED: %s\n", error);
}

int main(int argc, char* argv[])
{
    srand(time(NULL));

    char serial_port[128];
    Config config = initialize(argc, argv);
    
    if (config.hardware_type == EMULATOR) {
        if (z80aw_initialize_emulator(".", serial_port, sizeof serial_port, NULL) != 0) {
            fprintf(stderr, "Error initializing emulator: %s", z80aw_last_error());
            exit(1);
        }
    } else {
        strcpy(serial_port, config.serial_port);
    }
    
    printf("Serial port: %s\n", serial_port);
    
    z80aw_set_error_callback(error_cb, NULL);
    
    if (z80aw_init(serial_port) < 0) {
        fprintf(stderr, "%s\n", z80aw_last_error());
        return EXIT_FAILURE;
    }
    z80aw_set_logging_to_stdout(config.log_to_stdout);
    z80aw_set_assert_empty_buffer(false);
    
    uint8_t block[MAX_BLOCK_SIZE], rblock[MAX_BLOCK_SIZE];

    //
    // generic commands
    //
    
    ASSERT("Basic test", zsend_expect('A', 'a') == 0);

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
    
    uint16_t fr = z80aw_controller_info().free_memory;
    printf("Free memory in controller: %d bytes.\n", fr);
    ASSERT("Controller info - free memory", fr > 64);

    // 
    // power down CPU
    //

    ASSERT("Power down CPU", z80aw_cpu_powerdown() == 0);
    
    //
    // memory
    //
    
    uint8_t chk[] = { 0xfa, 0x80, 0x0, 0x79, 0xab };
    ASSERT("Checksum", z80aw_checksum(sizeof chk, chk) == 0x87a0);
    
    uint8_t byte = rand();
    ASSERT("Write byte", z80aw_write_byte(0x8, byte) == 0);
    ASSERT("Read byte", z80aw_read_byte(0x8) == byte);
    
    for (size_t unit = 0; unit < 2; ++unit) {
        printf("Memory unit %lu\n", unit + 1);
        for (size_t i = 0; i < MAX_BLOCK_SIZE; ++i)
            block[i] = (byte++) & 0xff;
        printf("Write block\n");
        int h = z80aw_write_block(0x100 + (unit * 0x8000), MAX_BLOCK_SIZE, block);
        // ASSERT("Write block", z80aw_write_block(0x100 + (unit * 0x8000), MAX_BLOCK_SIZE, block) == 0);
        ASSERT("Read block", z80aw_read_block(0x100 + (unit * 0x8000), MAX_BLOCK_SIZE, rblock) == 0);
        ASSERT("Compare blocks", memcmp(block, rblock, MAX_BLOCK_SIZE) == 0);

        if (h != 0) {
            printf("There was a check sum error when writing to memory. Here's the data.\n");
            for (int i = 0; i < MAX_BLOCK_SIZE; i += 0x20) {
                printf("%04lX : ", 0x100 + (unit * 0x8000) + i);
                printf("W "); for (int j = 0; j < 0x20; ++j) printf("%02X ", block[i + j]); printf("\n");
                printf("       R "); for (int j = 0; j < 0x20; ++j) printf("%s%02X\e[0m ", block[i + j] != rblock[i + j] ? "\e[0;33m" : "", rblock[i + j]); printf("\n");
            }
        }
    }

    // don't allow accessing memory with CPU powered on and not on BUSACK
    /* TODO - why is this not working?
    ASSERT("CPU reset", z80aw_cpu_reset() == 0);
    ASSERT("Don't allow write byte", z80aw_write_byte(0x8, byte) != 0);
    ASSERT("Don't allow read byte", z80aw_read_byte(0x8) < 0);
    z80aw_cpu_powerdown();
    */
    
    //
    // compiler
    //
    
    DebugInformation* di = compile_vasm("z80src/project.toml");
    ASSERT("DebugInformation is not null", di);
    ASSERT("Project type is correct", debug_project_type(di) == PT_VASM);
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

    // clear checksum
    uint8_t cc[2] = { 0, 0 };
    ASSERT("Clear checksum", z80aw_write_block(UPLOAD_CHECKSUM_LOCATION, 2, cc) == 0);
    
    // upload compiled code
    ASSERT("Check checksum (not uploaded)", !z80aw_is_uploaded(di));
    ASSERT("Upload successful", z80aw_upload_compiled(di, NULL, NULL) == 0);
    z80aw_read_block(0x0, debug_binary(di, 0)->sz, rblock);
    ASSERT("Test block 1 upload", memcmp(rblock, debug_binary(di, 0)->data, debug_binary(di, 0)->sz) == 0);
    ASSERT("Test block 2 upload", z80aw_read_byte(0x10) == 0xcf);
    z80aw_cpu_powerdown();
    ASSERT("Check checksum (uploaded)", z80aw_is_uploaded(di));

    debug_free(di);

    
    // simple compilation
    char errbuf[4096] = "";
    int resp = z80aw_simple_compilation(" jp 0xc3c3\n rst 0x8", errbuf, sizeof errbuf);
    ASSERT("Simple compilation", resp == 0);
    {
        uint8_t expected[] = { 0xc3, 0xc3, 0xc3, 0xcf };
        uint8_t found[4];
        z80aw_cpu_powerdown();
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
    ASSERT("PC == 0", z80aw_cpu_pc() == 0);

    // single nop
    z80aw_cpu_powerdown();
    z80aw_write_byte(0, 0);
    z80aw_write_byte(1, 0);
    z80aw_cpu_reset();
    ASSERT("Step (nop)", z80aw_cpu_step(NULL, NULL) == 0);
    ASSERT("PC == 0x1", z80aw_cpu_pc() == 0x1);
   
    // single step
    uint8_t jp[] = { 0xc3, 0xc3, 0xc3 };
    z80aw_cpu_powerdown();
    z80aw_write_block(0, sizeof jp, jp);
    z80aw_cpu_reset();
    ASSERT("Step (jp 0xc3c3)",z80aw_cpu_step(NULL, NULL) == 0);
    ASSERT("PC == 0xC3C3", z80aw_cpu_pc() == 0xc3c3);
    
    // compile and execute step
    COMPILE(" ld a, 0x42\n ld (0x8300), a");
    uint8_t expected_code[] = { 0x3e, 0x42, 0x32, 0x00, 0x83 };
    uint8_t compiled_code[sizeof expected_code];
    z80aw_read_block(0x0, sizeof expected_code, compiled_code);
    ASSERT("Code compiled correctly", memcmp(expected_code, compiled_code, sizeof expected_code) == 0);
    z80aw_cpu_reset();
    ASSERT("Step [0x8300] = 0x42", z80aw_cpu_step(NULL, NULL) == 0);
    z80aw_cpu_step(NULL, NULL);
    ASSERT("[0x8300] == 0x42", z80aw_read_byte(0x8300) == 0x42);
    
    // char on the screen
    COMPILE(" ld a, 'H'\n out (0), a\n nop");   // device 0x0 = video
    uint8_t c;
    z80aw_cpu_reset();
    z80aw_cpu_step(NULL, NULL);
    z80aw_cpu_step(NULL, &c);
    ASSERT("Char printed = 'H'", c == 'H');
    z80aw_cpu_step(NULL, &c);
    ASSERT("Print char is cleared", c == 0);
    
    // receive keypress
    COMPILE(" nop\n in a, (0x1)\n ld (0x8500), a\n nop");   // device 0x1 = keyboard
    z80aw_cpu_reset();
    z80aw_keypress('r');
    z80aw_cpu_step(NULL, NULL);
    z80aw_cpu_step(NULL, NULL);
    z80aw_cpu_step(NULL, NULL);
    z80aw_cpu_step(NULL, NULL);
    ASSERT("Receive keypress", z80aw_read_byte(0x8500) == 'r');
    
    // keypress interrupt
    COMPILE(" jp main\n"
            " org 0x8 \n"
            " in a, (0x1)\n"
            " ld (0x8400), a\n"
            " halt    \n"
            "main:    \n"
            " im 0    \n"
            " ei      \n"
            "cc: jp cc");
    z80aw_cpu_reset();
    for (size_t i = 0; i < 6; ++i)
        z80aw_cpu_step(NULL, NULL);
    z80aw_keypress('k');
    for (size_t i = 0; i < 6; ++i)
        z80aw_cpu_step(NULL, NULL);
    ASSERT("Keyboard interrupt was received", z80aw_read_byte(0x8400) == 'k');
    
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
    
    ASSERT("No last event", z80aw_last_event().char_printed == 0 && !z80aw_last_event().bkp_reached);
    
    // breakpoint hit
    COMPILE(" s: nop\n nop\n nop\n nop\n jp s");
    z80aw_cpu_reset();
    z80aw_add_breakpoint(0x3);
    ASSERT("Continue execution", z80aw_cpu_continue() == 0);
    while (!z80aw_last_event().bkp_reached);
    ASSERT("Stop at breakpoint", z80aw_cpu_pc() == 0x3);
    z80aw_remove_all_breakpoints();
    
    // keypress
    COMPILE(" jp main\n"
            " org 0x8 \n"
            " in a, (0x1)\n"
            " ld (0x8400), a\n"
            " halt    \n"
            "main:    \n"
            " im 0    \n"
            " ei      \n"
            "cc: jp cc");
    z80aw_cpu_reset();
    z80aw_add_breakpoint(0xa);
    z80aw_cpu_continue();
    usleep(100000);
    z80aw_keypress('f');
    while (!z80aw_last_event().bkp_reached);
    ASSERT("Key was pressed during continue", z80aw_cpu_pc() == 0xa);
    
    // stop
    z80aw_remove_all_breakpoints();
    z80aw_cpu_reset();
    z80aw_cpu_continue();
    usleep(10000);
    z80aw_cpu_stop();
    ASSERT("Stop stopped at the correct moment", z80aw_cpu_pc() == 0x11);
    
    /* TODO - reactivate test
    z80aw_cpu_continue();
    z80aw_keypress('g');
    usleep(10000);
    z80aw_cpu_stop();
    ASSERT("Stop stopped at the correct moment (after interrupt)", z80aw_read_byte(0x8400) == 'g');
    */
    
    // fetch registers
    if (config.hardware_type == EMULATOR) {
        Z80AW_Registers reg;
        ASSERT("Fetching registers from emulator", z80aw_cpu_registers(&reg) == 0);
        ASSERT("Check that registers were loaded correctly", (reg.AF >> 8) == 'g');
    }
    
    // print
    COMPILE(" ld a, 'A'\n"
            " out (0), a\n"      // device 0x0 = video
            " ld a, 'W' \n"
            " out (0), a\n"
            "x: jp x    \n");
    z80aw_cpu_reset();
    z80aw_cpu_continue();
    usleep(10000);
    Z80AW_Event e = z80aw_last_event();
    ASSERT("Check that character 'A' was printed", e.char_printed == 'A');
    z80aw_cpu_continue();
    usleep(10000);
    e = z80aw_last_event();
    ASSERT("Check that character 'W' was printed", e.char_printed == 'W');
    z80aw_cpu_stop();
    
    //
    // next
    //
    COMPILE(" nop            \n"
            " call sr        \n"
            " nop            \n"
            " sr: ld a, 0x68 \n"
            " ld (0x8800), a \n"
            " ret            \n");
    z80aw_cpu_reset();
    z80aw_cpu_next();
    usleep(10000);
    ASSERT("Next: last event after regular opcode", z80aw_last_event().bkp_reached);
    z80aw_cpu_next();
    usleep(10000);
    ASSERT("Next: last event after subrouting opcode", z80aw_last_event().bkp_reached);
    ASSERT("Next: returned from subroutine", z80aw_cpu_pc() == 0x4);
    ASSERT("Next: memory was set correctly", z80aw_read_byte(0x8800) == 0x68);

    // 
    // set and read high memory location
    //
    COMPILE(" ld a, 0x78\n ld (0xfe00), a");
    z80aw_cpu_reset();
    z80aw_cpu_step(NULL, NULL);
    z80aw_cpu_step(NULL, NULL);
    ASSERT("High memory working fine", z80aw_read_byte(0xfe00) == 0x78);

    //
    // test stack
    //
    COMPILE(" ld sp, 0xfffe\n ld bc, 0x1234\n push bc");
    z80aw_cpu_reset();
    z80aw_cpu_step(NULL, NULL);
    z80aw_cpu_step(NULL, NULL);
    z80aw_cpu_step(NULL, NULL);
    ASSERT("Stack working fine", z80aw_read_byte(0xfffd) == 0x12);

    //
    // test jump
    //
    char code_buf[16 * 1024];
    snprintf(code_buf, sizeof code_buf,
             "  ld   sp, 0xfffe         \n"   // 00
             "  jp   main               \n"   // 03
             "  org  0x66               \n"
             "  halt                    \n"   // 66
             "main:                     \n"
             "  ld   a, 0xfd            \n"   // 67
             "  ld   (0x9400), a        \n"   // 69
             "hng: jp hng               \n"); // 6c
    COMPILE(code_buf);
    z80aw_cpu_reset();
    for (int i = 0; i < 8; ++i) {
        if (log_to_stdout)
            printf(" [PC = 0x%x] ", z80aw_cpu_pc());
        z80aw_cpu_step(NULL, NULL);
    }
    ASSERT("Jumps are working fine", z80aw_read_byte(0x9400) == 0xfd);
    
    //
    // test EI instruction
    //
    /* TODO - why EI instruction doesn't stop in the emulator?
    COMPILE(" ei\n nop\n nop");
    z80aw_cpu_reset();
    z80aw_cpu_step(NULL, NULL);
    ASSERT("EI only uses one instruction", z80aw_cpu_pc() == 0x1);
    */

    //
    // let simple OS loaded into the memory
    //
    DebugInformation* dd = compile_vasm("z80src/simple/simple.toml");
    z80aw_upload_compiled(dd, NULL, NULL);
    debug_free(dd);
    
    //
    // breakpoint at even point
    //
    z80aw_cpu_reset();
    z80aw_add_breakpoint(0x16);
    ASSERT("Querying breakpoints", z80aw_query_breakpoints(bkps, 16) == 1);
    ASSERT("Checking brekpoints", bkps[0] == 0x16);
    z80aw_cpu_continue();
    while (!z80aw_last_event().bkp_reached);
    ASSERT("Stop at breakpoint (simple OS)", z80aw_cpu_pc() == 0x16);
    z80aw_remove_all_breakpoints();
    
    //
    // finalize
    //
    z80aw_finalize_emulator();
    
    z80aw_close();
}

// vim: set ts=4 sts=4 sw=4 expandtab:
