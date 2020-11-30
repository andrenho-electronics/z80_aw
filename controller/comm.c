#include "comm.h"

#include <stdint.h>

#include "io.h"
#include "messages.h"
#include "serial.h"

void comm_listen()
{
    uint8_t cmd = serial_recv();
    switch (cmd) {
        case ENQUIRY:
            serial_send(ACK);
            break;
        case STATUS: {
                uint16_t addr;
                uint8_t data;
                Inputs inputs;
                io_read_inputs(&addr, &data, &inputs);
                serial_send16(addr);
                serial_send(data);
                serial_send(*(uint8_t*) &inputs);
            }
            break;
        default:
            serial_send(INVALID_COMMAND);
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
