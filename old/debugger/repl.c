#include "repl.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>

#include "io.h"
#include "serial.h"

static const char* H_HELP = "h: help\r\n";
static const char* W_HELP = "w ADDR DATA: write byte to memory\r\n";
static const char* R_HELP = "r ADDR: read byte from memory\r\n";
static const char* L_HELP = "l ADDR: print 256 bytes starting from ADDR\r\n";
static const char* U_HELP = "u SIZE: enter programming mode\r\n";

static int prog_size = 0;

static void
print_help()
{
    ser_printstr(H_HELP);
    ser_printstr(W_HELP);
    ser_printstr(R_HELP);
    ser_printstr(L_HELP);
    ser_printstr(U_HELP);
    ser_printstr("data format is uppercase hexa (ex. 0C AF 12)\r\n");
}

static void
list(uint16_t addr)
{
    for (uint16_t a = addr; a < (addr + 256); a += 8) {
        char buf[80];
        uint8_t data[8];
        for (int i=0; i<8; ++i)
            data[i] = io_read(a + i);
#define CHR(i) ((data[i] >= 32 && data[i] < 127) ? data[i] : '.')
        snprintf(buf, sizeof buf, "%04X : %02X %02X %02X %02X  %02X %02X %02X %02X  %c%c%c%c%c%c%c%c\r\n", a,
                data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], 
                CHR(0), CHR(1), CHR(2), CHR(3), CHR(4), CHR(5), CHR(6), CHR(7));
#undef CRH
        ser_printstr(buf);
    }
}

void
repl_do()
{
    int pars;
    unsigned data1, data2;
    if (prog_size == 0) {
        // ser_printstr("? ");
        switch (ser_input(&data1, &data2, &pars)) {
            case 'h':
                print_help();
                return;
            case 'w':
                if (pars == 2)
                    io_write(data1, data2);
                else
                    ser_printstr(W_HELP);
                return;
            case 'r':
                if (pars == 1)
                    ser_printhex(io_read(data1), 2);
                else
                    ser_printstr(R_HELP);
                return;
            case 'u':
                if (pars == 1) {
                    prog_size = data1;
                    ser_printstr("ok\r\n");
                } else {
                    ser_printstr(U_HELP);
                }
                return;
            case 'l':
                if (pars == 1)
                    list((data1 / 16) * 16);
                else
                    ser_printstr(L_HELP);
                return;
            case 0:
                return;
        }
        ser_printstr("Syntax error.\r\n");
    } else {
        for (int i = 0; i < prog_size; ++i) {
            io_write(i, ser_inputhex());
            _delay_ms(1);
        }
        _delay_ms(100);
        for (int j = 0; j < prog_size; ++j) {
            ser_printhex(io_read(j), 2);
            _delay_us(100);
        }
        prog_size = 0;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
