#include "dbg.h"

#include <stddef.h>

#include "../common/protocol.h"
#include "breakpoints.h"
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
        case Z_STEP: {
                uint8_t printed_char = z80_step();
                serial_send(printed_char);
            }
            break;
        case Z_KEYPRESS:
            z80_set_last_keypress(serial_recv());
            z80_interrupt(0xcf);   // RST 0x8  
            serial_send(Z_OK);
            break;

        //
        // breakpoints
        //

        case Z_ADD_BKP:
            if (bkp_add(serial_recv16()))
                serial_send(Z_OK);
            else
                serial_send(Z_TOO_MANY_BKPS);
            break;
        case Z_REMOVE_BKP:
            bkp_remove(serial_recv16());
            serial_send(Z_OK);
            break;
        case Z_REMOVE_ALL_BKPS:
            bkp_remove_all();
            serial_send(Z_OK);
            break;
        case Z_QUERY_BKPS: {
                uint16_t bkps[MAX_BREAKPOINTS];
                int count = bkp_query(bkps);
                serial_send(count);
                for (int i = 0; i < count; ++i)
                    serial_send16(bkps[i]);
            }
            break;

        //
        // continue
        //

        case Z_LAST_EVENT:
            switch (z80_last_event()) {
                case E_NO_EVENT:
                    serial_send(Z_OK);
                    break;
                case E_BREAKPOINT_HIT:
                    serial_send(Z_BKP_REACHED);
                    break;
                default:
                    serial_send(Z_INVALID_CMD);
            }
            /*
            if (last_event == Z_PRINT_CHAR)
                send(last_printed_char);
            last_event = Z_OK;
            */
            break;

        case Z_CONTINUE:
            z80_continue();
            serial_send(Z_OK);
            return;

        // 
        // not matching
        //

        default:
            serial_send(Z_INVALID_CMD);
            break;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
