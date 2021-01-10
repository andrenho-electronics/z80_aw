#include "dbg.h"

#include <stddef.h>

#include "../common/protocol.h"
#include "memory.h"
#include "serial.h"
#include "util.h"
#include "z80.h"

void debugger_cycle()
{
    uint8_t data[512] = { 0 };
    uint8_t c = serial_recv();

    switch (c) {
        // 
        // controller
        //

        case 'A':
            serial_send('a');   // this is for manual tests
            break;
        case Z_ACK_REQUEST:
            serial_send(Z_ACK_RESPONSE);
            break;
        case Z_CTRL_INFO:
            serial_send16(free_ram());
            break;

        // 
        // memory
        //

        case Z_WRITE_BLOCK: {
                uint16_t addr = serial_recv16();
                uint16_t sz = serial_recv16();
                for (size_t i = 0; i < sz; ++i)
                    data[i] = serial_recv();
                uint16_t checksum;
                if (memory_write_page(addr, data, sz, &checksum))
                    serial_send(Z_OK);
                else
                    serial_send(Z_INCORRECT_BUS);
                serial_send16(checksum);
            }
            break;
        case Z_READ_BLOCK: {
                uint16_t addr = serial_recv16();
                uint16_t sz = serial_recv16();
                if (memory_read_page(addr, data, sz))
                    serial_send(Z_OK);
                else
                    serial_send(Z_INCORRECT_BUS);
                for (size_t i = 0; i < sz; ++i)
                    serial_send(data[i]);
            }
            break;

        //
        // cpu
        //

        case Z_POWERDOWN:
            z80_powerdown();
            serial_send(Z_OK);
            break;
        case Z_RESET:
            z80_reset();
            serial_send(Z_OK);
            break;
        case Z_PC:
            serial_send16(z80_pc());
            break;

        //
        // breakpoints
        //

        case Z_REMOVE_ALL_BKPS:
            serial_send(Z_OK);  // TODO
            break;

        // 
        // not matching
        //

        default:
            serial_send(Z_INVALID_CMD);
            break;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
