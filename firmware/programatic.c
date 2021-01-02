#include "programatic.h"

#include "bus.h"
#include "memory.h"
#include "protocol.h"
#include "serial.h"
#include "z80.h"

static void programatic_upload();

void programatic_command(uint8_t c)
{
    switch (c) {
        case C_ACK:
            serial_send(C_OK);
            break;
        case C_REGISTERS:
            for (size_t i = 0; i < 20; ++i)  // TODO - send rest of the registers
                serial_send(0);
            serial_send(z80_last_pc & 0xff);
            serial_send(z80_last_pc >> 8);
            for (size_t i = 0; i < 5; ++i)
                serial_send(0);
            break;
        case C_RAM_BYTE: {
                uint8_t a = serial_recv();
                uint8_t b = serial_recv();
                serial_send(memory_read(a | (b << 8)));
            }
            break;
        case C_RAM_BLOCK: {
                uint8_t a = serial_recv();
                uint8_t b = serial_recv();
                uint8_t c1 = serial_recv();
                uint8_t c2 = serial_recv();
                uint16_t addr = a | (b << 8);
                uint16_t sz = c1 | (c2 << 8);
                for (uint16_t i = 0; i < sz; ++i)
                    serial_send(memory_read(addr + i));
            }
            break;
        case C_UPLOAD:
            programatic_upload();
            break;
        case C_STEP:
            {
                uint16_t pc = z80_step();
                serial_send(pc & 0xff);
                serial_send(pc >> 8);
                serial_send(0); // TODO - print char
            }
            break;
        case C_RESET:
            z80_powerdown();
            z80_init();
            z80_step();
            serial_send(C_OK);
            break;
        default:
            serial_send(C_ERR);
            for(;;);
    }
}

static void programatic_upload()
{
    z80_powerdown();
    z80_clock_cycle(true);

    // respond with ack
    if (z80_controls_bus()) {
        serial_send(C_UPLOAD_ERROR);
        return;
    } else {
        serial_send(C_UPLOAD_ACK);
    }

    for (;;) {
        // receive inital address
        uint16_t addr = serial_recv16();
        serial_send(C_UPLOAD_ACK);

        // receive size
        uint8_t block_size = serial_recv16();
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
        uint8_t checksum1 = 0, checksum2 = 0;
        for (size_t i = 0; i < block_size; ++i) {
            checksum1 = (checksum1 + rdata[i]) % 255;
            checksum2 = (checksum2 + checksum1) % 255;
        }
        serial_send(checksum1);
        serial_send(checksum2);
    }

    serial_send(C_UPLOAD_ACK);
}

// vim:ts=4:sts=4:sw=4:expandtab
