#include "dbg.h"

#include "lowlevel.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"
#include "../common/protocol.h"


void dbg_exec()
{
    uint8_t data[512] = { 0 };
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
        case Z_WRITE_BLOCK: {
                uint16_t addr = serial_recv16();
                uint16_t sz = serial_recv16();
                for (size_t i = 0; i < sz; ++i)
                    data[i] = serial_recv();
                int checksum = memory_write_page(addr, data, sz);
                serial_send16(checksum);
            }
            break;
        case Z_READ_BLOCK: {
                uint16_t addr = serial_recv16();
                uint16_t sz = serial_recv16();
                memory_read_page(addr, data, sz);
                for (size_t i = 0; i < sz; ++i)
                    serial_send(data[i]);
            }
            break;
        case Z_REMOVE_ALL_BKPS:
            serial_send(Z_OK);  // TODO
            break;
        case Z_RESET:
            z80_init();
            serial_send(Z_OK);
            break;
        case Z_POWERDOWN:
            z80_powerdown();
            serial_send(Z_OK);
            break;
        case Z_PC:
            serial_send16(z80_last_pc);
            break;
        case Z_STEP: {
                uint8_t printed_char = z80_step();
                serial_send(printed_char);
            }
            break;
        case Z_KEYPRESS:
            z80_keypress(serial_recv());
            serial_send(Z_OK);
            break;
        default:
            serial_send(Z_INVALID_CMD);
            break;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
