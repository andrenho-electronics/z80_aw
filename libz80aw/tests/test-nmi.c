#include <stdio.h>
#include <stdlib.h>

#include "../comm/z80aw.h"
#include "../comm/compiler.h"

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

static void error_cb(const char* error, void* data)
{
    (void) data;
    fprintf(stderr, "ERROR DETECTED: %s\n", error);
    exit(EXIT_FAILURE);
}


int main(int argc, char* argv[])
{
    z80aw_set_error_callback(error_cb, NULL);

    char serial_port[128];
    if (argc == 1) {
        if (z80aw_initialize_emulator(".", serial_port, sizeof serial_port) != 0) {
            fprintf(stderr, "Error initializing emulator: %s", z80aw_last_error());
            exit(1);
        }
        z80aw_init(serial_port);
    } else {
        z80aw_init(argv[1]);
    }

    z80aw_set_logging_to_stdout(true);
    z80aw_set_assert_empty_buffer(true);

    //
    // load registers from Z80 code
    //
    
    FILE* f = fopen("tests/registers.z80", "r");
    ASSERT("Open register asm file", f);
    char reg_buf[8 * 1024];
    fread(reg_buf, sizeof reg_buf, 1, f);
    fclose(f);
    
    char code_buf[16 * 1024];
    snprintf(code_buf, sizeof code_buf,
             "  ld   sp, 0xfffe         \n"
             "  jp   main               \n"
             "  org  0x66               \n"   // NMI
             "  jp   debugger_registers \n"
             "main:                     \n"
             "  ld   a, 0xa             \n"
             "  ld   bc, 0xbc           \n"
             "  ld   de, 0x0            \n"
             "  ld   hl, 0x0            \n"
             "  ex   af, af'            \n"
             "  exx                     \n"
             "  ld   bc, 0              \n"
             "  ld   hl, 0x41           \n"
             "  ld   a, 0x1             \n"
             "  ld   i, a               \n"
             "  ld   de, 0xde           \n"
             "  ld   ix, 0x0            \n"
             "  ld   iy, 0x9f           \n"
             "cc: jp cc                 \n"
             "%s", reg_buf);
    COMPILE(code_buf);

    printf("Preparing NMI register fetch mode...\n");
    z80aw_set_register_fetch_mode(Z80AW_REGFETCH_NMI);
    printf("Done.\n");
    
    Z80AW_Registers r;
    z80aw_cpu_reset();
    for (int i = 0; i < 32; ++i) {
        printf(" [PC = 0x%x] ", z80aw_cpu_pc());
        z80aw_cpu_step(NULL, NULL);
    }
    uint16_t original_pc = z80aw_cpu_pc();
    ASSERT("Execute step debug", z80aw_cpu_step(&r, NULL) == 0);
    ASSERT("SP == 0xFFFE", r.SP == 0xfffe);
    ASSERT("A' == 0xA", (r.AFx >> 8) == 0xa);
    ASSERT("BC' == 0xBC", r.BCx == 0xbc);
    ASSERT("HL == 0xHL", r.HL == 0x41);
    ASSERT("A == 0x1", (r.AF >> 8) == 0x1);
    ASSERT("I == 0x1", r.I == 0x1);
    ASSERT("DE == 0xDE", r.DE == 0xde);
    ASSERT("IY == 0x9F", r.IY == 0x9f);
    uint16_t new_pc = z80aw_cpu_pc();
    ASSERT("Returned to the next PC", new_pc == original_pc);

    z80aw_close();

    return EXIT_SUCCESS;
}
