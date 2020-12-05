#include "test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define MAGENTA "\033[35m"
#define RESET   "\033[0m"

static void test_memory(int displacement, CommLib* cl)
{
    uint8_t byte = 0;

    uint16_t addr = 0 + (displacement * 0x8000);
    for (uint16_t i = 0; i < 0x8000; i += 0x7000) {
        for (uint16_t j = 0; j < 0x80; j += 0x10) {
            printf("%04X   ", addr + i + j);

            // write
            uint8_t data_w[0x10];
            for (uint8_t k = 0; k < 0x10; ++k)
                data_w[k] = byte++;
            cl_write_memory(cl, addr + i + j, data_w, 0x10);

            // read
            uint8_t data_r[0x10];
            cl_read_memory(cl, addr + i + j, data_r, 0x10);
            for (uint8_t k = 0; k < 0x10; ++k)
                printf("%s", data_r[k] == data_w[k] ? GREEN "." RESET : RED "X" RESET);
            printf("\n");
        }
    }
}

void command_test(const char* line, CommLib* cl)
{
    char buf[21];
    int n = sscanf(line, " %20s", buf);

    if (n == 1 && strcmp(buf, "rom") == 0)
        test_memory(0, cl);
    else if (n == 1 && strcmp(buf, "ram") == 0)
        test_memory(1, cl);
    else
        command_help(" test", cl);
}

// vim:ts=4:sts=4:sw=4:expandtab
