#include <stdio.h>
#include <stdlib.h>

#include "../comm/z80aw.h"

static void error_cb(const char* description, void* data)
{
    (void) data;
    fprintf(stderr, "Error: %s\n", description);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        printf("Usage: %s SERIAL_PORT PAGE_HEX\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    uint8_t page[0x100];
    uint8_t npage = strtoul(argv[2], NULL, 16);

    z80aw_set_error_callback(error_cb, NULL);

    z80aw_init(argv[1]);
    z80aw_cpu_powerdown();
    z80aw_read_block(npage * 0x100, 0x100, page);
    z80aw_close();

    printf("        _0 _1 _2 _3 _4 _5 _6 _7 _8 _9 _A _B _C _D _E _F\n");
    for (uint16_t a = 0; a < 0x100; a += 0x10) {
        printf("%04X : ", (npage * 0x100) + a);
        for (uint16_t b = 0; b < 0x10; ++b)
            printf("%02X ", page[a + b]);
        printf("   ");
        for (uint16_t b = 0; b < 0x10; ++b) {
            char c = page[a + b];
            printf("%c", c >= 32 && c < 127 ? c : '.');
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}
