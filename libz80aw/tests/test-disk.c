#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../comm/z80aw.h"

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

bool mlog_to_stdout = false;
char* serial = NULL;

static void print_help()
{
    printf("Usage:\n");
    printf("   -h               This help.\n");
    printf("   -l               Log to stdout.\n");
    printf("   -r SERIAL_PORT   Use real hardware.\n");
}

static void get_options(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "hlr:")) != -1) {
        switch (opt) {
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            case 'l':
                mlog_to_stdout = true;
                break;
            case 'r':
                serial = optarg;
                break;
            default:
                print_help();
                exit(EXIT_FAILURE);
        }
    }
}

static void error_cb(const char* error, void* data)
{
    (void) data;
    fprintf(stderr, "ERROR DETECTED: %s\n", error);
}

int main(int argc, char* argv[])
{
    get_options(argc, argv);
    
    SDCardStage disk_stage;
    uint8_t disk_status;
    
    //
    // compile SDCARD disk
    //
    DebugInformation* di = compile_vasm("z80src/sdcard/sdcard.toml");
    ASSERT("DebugInformation is not null", di);
    ASSERT("Project type is correct", debug_project_type(di) == PT_VASM_DISK);
    ASSERT("Compiler output is successful", debug_output(di, NULL, 0));
    if (mlog_to_stdout) {
        printf("Compiler output:\n\e[0;33m");
        debug_print(di);
        printf("\e[0m\n");
    }
    
    // generate image
    if (!serial) {
        unlink("/tmp/sdcard.img");
        ASSERT("Generate image", debug_generate_image(di, "/tmp/sdcard.img") == 0);
        if (mlog_to_stdout) {
            printf("Image files:\n\e[0;33m");
            system("mdir -i /tmp/sdcard.img ::");
            printf("\e[0m\n");
        }
    }
    
    //
    // test in emulator
    //
    
    // initialize
    z80aw_set_error_callback(error_cb, NULL);
    z80aw_set_logging_to_stdout(mlog_to_stdout);
    
    char serial_port[128];
    if (!serial) {
        if (z80aw_initialize_emulator(".", serial_port, sizeof serial_port, "/tmp/sdcard.img") != 0) {
            fprintf(stderr, "Error initializing emulator: %s", z80aw_last_error());
            exit(1);
        }
    } else {
        strcpy(serial_port, serial);
    }
    
    if (z80aw_init(serial_port) < 0) {
        fprintf(stderr, "%s\n", z80aw_last_error());
        return EXIT_FAILURE;
    }
    ASSERT("Get disk status", z80aw_disk_status(&disk_stage, &disk_status) == 0);
    ASSERT("Disk stage == SD_WRITE_OK (from writing to boot sector)", disk_stage == SD_WRITE_OK);
    ASSERT("Disk status == 0x0", disk_status == 0x0);
    
    // read first 1k
    uint8_t data[1024];
    FILE* f = fopen("/tmp/sdcard.img", "r");
    fread(data, 1024, 1, f);
    fclose(f);
    
    // read SD data through protocol
    uint8_t block[512];
    ASSERT("Read first block", z80aw_read_disk_block(0, block) == 0);
    if (!serial) {
        ASSERT("Check that first block is correct", memcmp(block, data, 512) == 0);
    }
    ASSERT("Read second block", z80aw_read_disk_block(1, block) == 0);
    if (!serial) {
        ASSERT("Check that second block is correct", memcmp(block, &data[512], 512) == 0);
    }
    
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
    
    // run code for reading from disk
    COMPILE(" ld bc, 0x0        ; disk block 1st and 2nd bytes \n"
            " ld (0x8000), bc \n"
            " ld bc, 0x0        ; disk block 3rd and 4th byte \n"
            " ld (0x8002), a \n"
            " ld bc, 0xf000     ; memory destination\n"
            " ld (0x8004), bc \n"
            " ld bc, 0x2        ; number of blocks\n"
            " ld (0x8006), bc\n"
            " ld a, 0x0         ; read structure low address byte\n"
            " out (0x2), a\n"
            " ld a, 0x80        ; read structure high address byte\n"
            " out (0x3), a\n"
            "hg: jp hg");
    z80aw_cpu_reset();
    for (int i = 0; i < 16; ++i)
        z80aw_cpu_step(NULL, NULL);
    uint8_t rdata[1024];
    z80aw_read_block(0xf000, 1024, rdata);
    ASSERT("OUT command for reading", memcmp(data, rdata, 1024) == 0);
    
    ASSERT("Get disk status", z80aw_disk_status(&disk_stage, &disk_status) == 0);
    ASSERT("Disk stage == SD_READ_OK", disk_stage == SD_READ_OK);
    ASSERT("Disk status == 0x0", disk_status == 0x0);
    
    // run code for writing to disk
    uint8_t expected[512];
    for (int i = 0; i < 512; ++i)
        expected[i] = i & 0xff;
    
    COMPILE(" ld b, 0x0      \n"        // write 512 sequential bytes to 0xE000
            " ld de, 512   \n"
            " ld hl, 0xe000\n"
            "loop:         \n"
            " ld (hl), b   \n"
            " inc hl       \n"
            " inc b        \n"
            " dec de       \n"
            " ld a, d      \n"
            " or e         \n"
            " jr nz, loop   \n"
            // write to SD Card
            " ld bc, 0x8        ; disk block 1st and 2nd bytes \n"
            " ld (0x8000), bc   \n"
            " ld bc, 0x0        ; disk block 3rd and 4th byte \n"
            " ld (0x8002), a    \n"
            " ld bc, 0xe000     ; memory origin\n"
            " ld (0x8004), bc    \n"
            " ld bc, 0x1        ; number of blocks\n"
            " ld (0x8006), bc   \n"
            " ld a, 0x0         ; read structure low address byte\n"
            " out (0x4), a      \n"
            " ld a, 0x80        ; read structure high address byte\n"
            " out (0x5), a      \n"
            "  ld a, 0xff       ; stop condition\n"
            "  ld (0x3000), a   \n"
            "hg: jp hg");
    
    z80aw_cpu_reset();
    while (z80aw_read_byte(0x3000) != 0xff)
        z80aw_cpu_step(NULL, NULL);
    
    z80aw_read_block(0xe000, 512, block);
    ASSERT("Check that our logic is correct", memcmp(expected, block, 512) == 0);
    
    uint8_t sdw[512];
    z80aw_read_disk_block(8, sdw);
    ASSERT("OUT command for writing", memcmp(expected, sdw, 512) == 0);
    
    ASSERT("Get disk status", z80aw_disk_status(&disk_stage, &disk_status) == 0);
    ASSERT("Disk stage == SD_READ_OK", disk_stage == SD_WRITE_OK);
    ASSERT("Disk status == 0x0", disk_status == 0x0);
    
    // finalize
    z80aw_close();
    unlink("/tmp/sdcard.img");
    debug_free(di);
}

