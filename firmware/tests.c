#include "tests.h"

#include <util/delay.h>

#include "memory.h"
#include "serial.h"

#if ADD_TESTS

static uint8_t counter = 0;
static uint8_t starter = 0;

static void test_memory_block(uint16_t start_addr)
{
    uint8_t data[0x20];
    for (uint16_t addr = start_addr; addr < start_addr + 0x100; addr += 0x20) {
        serial_printhex16(addr);
        serial_spaces(2);
        for (uint8_t a = 0; a < 0x20; ++a)
            data[a] = counter++;
        for (uint16_t j = 0; j < 0x20; ++j) {
            memory_write(addr + j, data[j], false);
            if (addr + j < 0x8000)
                _delay_ms(12);
        }
        for (uint16_t j = 0; j < 0x20; ++j) {
            uint8_t new_data = memory_read(addr + j);
            serial_printbit(new_data == data[j]);
        }
        serial_puts();
    }
}

static void test_memory()
{
    serial_printstr(PSTR("Testing ROM...\r\n"));
    counter = starter++;
    test_memory_block(0x0);
    test_memory_block(0x7e00);
    serial_printstr(PSTR("Testing RAM...\r\n"));
    counter = starter++;
    test_memory_block(0x8000);
}

void tests_run()
{
    test_memory();
}

#endif

// vim:ts=4:sts=4:sw=4:expandtab
