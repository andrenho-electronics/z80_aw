#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "../z80aw.h"
#include "../compiler.h"
#include "protocol.h"
#include "../comm.h"
#include "testutil.h"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31m❌\e[0m\n"); exit(1); }

int main(int argc, char* argv[])
{
    Config config = initialize(argc, argv);
    printf("Serial port: %s\n", config.serial_port);
    
    Z80AW_Config cfg = {
            .serial_port = config.serial_port,
            .log_to_stdout = config.log_to_stdout,
    };
    z80aw_init(&cfg);
    
    //
    // compiler
    //
    DebugInformation* di = compile_vasm("z80src/project.toml");
    ASSERT("DebugInformation is not null", di);
    ASSERT("Compiler output is successful", debug_output(di, NULL, 0));
    printf("Compiler output:\n\e[0;33m");
    debug_print(di);
    printf("\e[0m\n");
    
    DebugInformation* di_error = compile_vasm("z80src/project_error.toml");
    ASSERT("DebugInformation is not null", di_error);
    {
        char error_msg[4096];
        ASSERT("Compiler output is not successful", !debug_output(di_error, error_msg, sizeof error_msg));
        printf("Compiler error output:\n");
        printf("\e[0;33m%s\e[0m\n\n", error_msg);
    }
    debug_free(di_error);
    
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
    
    uint8_t block[MAX_BLOCK_SIZE], rblock[MAX_BLOCK_SIZE];
    for (size_t i = 0; i < MAX_BLOCK_SIZE; ++i)
        block[i] = (i + 1) & 0xff;
    ASSERT("Write block", z80aw_write_block(0x100, MAX_BLOCK_SIZE, block) == 0);
    ASSERT("Read block", z80aw_read_block(0x100, MAX_BLOCK_SIZE, rblock) == 0);
    ASSERT("Compare blocks", memcmp(block, rblock, MAX_BLOCK_SIZE) == 0);
    
    //
    // finalize
    //
    ASSERT("Finalizing emulator", zsend_expect(Z_EXIT_EMULATOR, Z_OK) == 0);
    
    debug_free(di);
    z80aw_close();
}