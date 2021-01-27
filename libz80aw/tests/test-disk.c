#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../comm/z80aw.h"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31mX\e[0m\n"); exit(1); }

bool mlog_to_stdout = false;

static void error_cb(const char* error, void* data)
{
    (void) data;
    fprintf(stderr, "ERROR DETECTED: %s\n", error);
}

int main(int argc, char* argv[])
{
    if (argc == 2 && strcmp(argv[1], "-l") == 0)
        mlog_to_stdout = true;
    
    //
    // compile SDCARD disk
    //
    DebugInformation* di = compile_vasm_disk("z80src/sdcard/sdcard.toml");
    ASSERT("DebugInformation is not null", di);
    ASSERT("Compiler output is successful", debug_output(di, NULL, 0));
    if (mlog_to_stdout) {
        printf("Compiler output:\n\e[0;33m");
        debug_print(di);
        printf("\e[0m\n");
    }
    
    // generate image
    unlink("/tmp/sdcard.img");
    ASSERT("Generate image", debug_generate_image(di, "/tmp/sdcard.img") == 0);
    if (mlog_to_stdout) {
        printf("Image files:\n\e[0;33m");
        system("mdir -i /tmp/sdcard.img ::");
        printf("\e[0m\n");
    }
    
    //
    // test in emulator
    //
    
    // initialize
    z80aw_set_error_callback(error_cb, NULL);
    z80aw_set_logging_to_stdout(mlog_to_stdout);
    
    char serial_port[128];
    if (z80aw_initialize_emulator(".", serial_port, sizeof serial_port, "/tmp/sdcard.img") != 0) {
        fprintf(stderr, "Error initializing emulator: %s", z80aw_last_error());
        exit(1);
    }
    if (z80aw_init(serial_port) < 0) {
        fprintf(stderr, "%s\n", z80aw_last_error());
        return EXIT_FAILURE;
    }
    
    // read first 1k
    uint8_t data[1024];
    FILE* f = fopen("/tmp/sdcard.img", "r");
    fread(data, 1024, 1, f);
    fclose(f);
    
    // read SD data through protocol
    uint8_t block[512];
    ASSERT("Read first block", z80aw_read_disk_block(0, block) == 0);
    ASSERT("Check that first block is correct", memcmp(block, data, 512) == 0);
    ASSERT("Read second block", z80aw_read_disk_block(1, block) == 0);
    ASSERT("Check that second block is correct", memcmp(block, &data[512], 512) == 0);
    
    // check memory
    ASSERT("Bootloader was added to the memory", z80aw_read_byte(0x0) == 0xc3);  // JP
    
    // run bootloader
    z80aw_set_register_fetch_mode(Z80AW_REGFETCH_EMULATOR);
    Z80AW_Registers r;
    z80aw_cpu_reset();
    z80aw_cpu_step(&r, NULL);
    z80aw_cpu_step(&r, NULL);
    ASSERT("Check that bootloader run correctly", (r.AF >> 8) == 0x64);
    
    // write a block into the emulator
    for (int i = 0; i < 512; ++i)
        block[i] = i & 0xff;
    ASSERT("Write block", z80aw_write_disk_block(5, block) == 0);
    uint8_t rblock[512];
    z80aw_read_disk_block(5, rblock);
    ASSERT("Read written block", memcmp(block, rblock, 512) == 0);
    
    // finalize
    z80aw_close();
    unlink("/tmp/sdcard.img");
    debug_free(di);
}

