#include "dbg.h"

#include "../common/protocol.h"
#include "serial.h"
#include "util.h"

void debugger_cycle()
{
    // uint8_t data[512] = { 0 };
    uint8_t c = serial_recv();

    switch (c) {
        case 'A':
            serial_send('a');   // this is for manual tests
            break;
        case Z_ACK_REQUEST:
            serial_send(Z_ACK_RESPONSE);
            break;
        case Z_CTRL_INFO:
            serial_send16(free_ram());
            break;
        default:
            serial_send(Z_INVALID_CMD);
            break;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
