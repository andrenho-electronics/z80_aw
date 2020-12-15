#include "tests.h"

#include <util/delay.h>

#include "ansi.h"
#include "memory.h"
#include "serial.h"

#if ADD_TESTS

static uint8_t counter = 0;
static uint8_t starter = 0;

static void test_memory_block(uint16_t start_addr)
{
    uint8_t data[0x40];
    for (uint16_t addr = start_addr; addr < start_addr + 0x100; addr += 0x40) {
        serial_printhex16(addr);
        serial_spaces(2);
        for (uint8_t a = 0; a < 0x40; ++a)
            data[a] = counter++;
        for (uint16_t j = 0; j < 0x40; ++j) {
            memory_write(addr + j, data[j], false);
            if (addr + j < 0x8000)
                _delay_ms(10);
        }
        for (uint16_t j = 0; j < 0x40; ++j) {
            uint8_t new_data = memory_read(addr + j);
            serial_printbit(new_data == data[j]);
        }
        serial_puts();
    }
}

static void test_memory_page(uint16_t start_addr)
{
    uint8_t data[64];
    uint8_t new_data[64];
    for (uint16_t addr = start_addr; addr < start_addr + 0x100; addr += 64) {
        serial_printhex16(addr);
        serial_spaces(2);
        for (int i = 0; i < 64; ++i)
            data[i] = counter++;
        memory_write_page(addr, data, 64);
        memory_read_page(addr, new_data);
        for (uint16_t j = 0; j < 64; ++j) {
            serial_printbit(new_data[j] == data[j]);
            if (new_data[j] != data[j]) {
                serial_printstr(PSTR(ANSI_RED));
                serial_printhex8(new_data[j]);
                serial_printstr(PSTR(ANSI_RESET));
                serial_send(' ');
            }
        }
        serial_puts();
    }
}

static void test_memory()
{
    counter = starter++;

    serial_printstr(PSTR("Testing ROM (individual writes)...\r\n"));
    test_memory_block(0x0);
    serial_printstr(PSTR("Testing ROM (page writes)...\r\n"));
    test_memory_page(0x7e00);

    serial_printstr(PSTR("Testing RAM...\r\n"));
    test_memory_block(0x8000);
    serial_printstr(PSTR("Done.\r\n"));
}

void tests_run()
{
    test_memory();
}

#endif

// vim:ts=4:sts=4:sw=4:expandtab
