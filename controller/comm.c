#include "comm.h"

#include <stdint.h>

#include "io.h"
#include "messages.h"
#include "serial.h"

uint16_t cycle = 0;

void comm_listen()
{
    uint8_t cmd = serial_recv();
    switch (cmd) {
        case CMD_ENQUIRY:
            serial_send(ACK);
            break;
        case CMD_STATUS: {
                uint16_t addr;
                uint8_t data;
                Inputs inputs;
                io_read_inputs(&addr, &data, &inputs);
                serial_send16(cycle);
                serial_send16(addr);
                serial_send(data);
                serial_send(*(uint8_t*) &inputs);
            }
            break;
        case CMD_CYCLE:
            io_z80_clock();
            ++cycle;
            serial_send(ACK);
            break;
        case CMD_INIT:
            io_z80_init();
            cycle = 0;
            serial_send(ACK);
            break;
        case CMD_RESET:
            io_z80_reset();
            cycle = 0;
            serial_send(ACK);
            break;
        case CMD_READ: {
                uint16_t addr = serial_recv16();
                uint16_t sz = serial_recv16();
                for (uint16_t i = addr; i < addr + sz; ++i)
                    serial_send(io_read_memory(i));
            }
            break;
        case CMD_WRITE: {
                uint16_t addr = serial_recv16();
                uint16_t sz = serial_recv16();
                for (uint16_t i = addr; i < addr + sz; ++i)
                    io_write_memory(i, serial_recv());
                serial_send(ACK);
            }
            break;
        default:
            serial_send(INVALID_COMMAND);
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
