#include "programatic.h"

#include "memory.h"
#include "serial.h"
#include "z80.h"

#include <util/delay.h>

#define P_ACK   0x1
#define P_ERROR 0x2

void programatic_upload()
{
    z80_powerdown();

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

        uint16_t checksum1 = 0, checksum2 = 0;
        for (int i = 0; i < block_size; ++i) {
            uint8_t data = serial_recv();
            checksum1 = (checksum1 + data) % 255;
            checksum2 = (checksum2 + checksum1) % 255;
            memory_write(addr++, data, false);
            _delay_ms(10);
        }

        serial_send(checksum1);
        serial_send(checksum2);
    }

    // check if finish or ask for verification
    uint8_t r = serial_recv();
    if (r == 0x0) {
        serial_send(P_ACK);
    } else if (r == 0x1) {

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
}

// vim:ts=4:sts=4:sw=4:expandtab
