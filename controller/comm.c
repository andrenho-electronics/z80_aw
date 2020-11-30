#include "comm.h"

#include <stdint.h>

#include "messages.h"
#include "serial.h"

void comm_listen()
{
    uint8_t cmd = serial_recv();
    switch (cmd) {
        case ENQUIRY:
            serial_send(ACK);
            break;
        default:
            serial_send(NAK);
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
