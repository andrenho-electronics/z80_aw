#include "programatic.h"

#include "bus.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"

#if !SIMULATOR
#  include <util/delay.h>
#else
void _delay_ms(int ms);
#endif

#define P_ACK   0x1
#define P_ERROR 0x2

void programatic_upload()
{
    z80_powerdown();
    z80_clock_cycle(true);

    // respond with ack
    if (z80_controls_bus()) {
        serial_send(P_ERROR);
        return;
    } else {
        serial_send(P_ACK);
    }

    // receive inital address
    uint16_t initial_addr = serial_recv16();
    serial_send(P_ACK);

    // receive bytes
    uint16_t addr = initial_addr;
    while (1) {
        uint8_t block_size = serial_recv();
        if (block_size == 0)
            break;

        // receive bytes
        uint8_t data[64];
        for (int i = 0; i < block_size; ++i)
            data[i] = serial_recv();

        // write memory
        memory_write_page(addr, data, block_size);

        // read bytes
        uint8_t rdata[64];
        memory_read_page(addr, rdata, block_size);

        // calculate checksum
        uint16_t checksum1 = 0, checksum2 = 0;
        for (int i = 0; i < block_size; ++i) {
            checksum1 = (checksum1 + rdata[i]) % 255;
            checksum2 = (checksum2 + checksum1) % 255;
        }
        serial_send(checksum1);
        serial_send(checksum2);
    }

    // check if finish or ask for verification
    uint8_t r = serial_recv();
    if (r == 0x0) {
        serial_send(P_ACK);
    } 
    /* - reenable this if it's necessary to perform a rewrite
    else if (r == 0x1) {

        // send written bytes
        for (uint16_t a = initial_addr; a < addr; ++a)
            serial_send(memory_read(a));

        // receive correction bytes
        uint16_t correction_bytes = serial_recv16();
        for (uint16_t i = 0; i < correction_bytes; ++i) {
            uint16_t a = serial_recv16();
            uint8_t  d = serial_recv();
            memory_write(a, d, true);
            serial_send(P_ACK);
        }
    }
    */
}

// vim:ts=4:sts=4:sw=4:expandtab
