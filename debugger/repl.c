#include "repl.h"

#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>

#include "io.h"
#include "serial.h"

static const char* H_HELP = "h: help\r\n";
static const char* W_HELP = "w ADDR DATA: write byte to memory\r\n";
static const char* R_HELP = "r ADDR: read byte from memory\r\n";
static const char* U_HELP = "u SIZE: enter programming mode\r\n";

static int prog_size = 0;

static void
print_help()
{
    ser_printstr(H_HELP);
    ser_printstr(W_HELP);
    ser_printstr(R_HELP);
    ser_printstr(U_HELP);
    ser_printstr("data format is uppercase hexa (ex. 0C AF 12)\r\n");
}

void
repl_do()
{
    int pars;
    unsigned data1, data2;
    if (prog_size == 0) {
        ser_printstr("? ");
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
                if (pars == 1)
                    prog_size = data1;
                else
                    ser_printstr(U_HELP);
                return;
            case 'l':  // TODO - remove
                for (int i=0; i < 4; ++i)
                    ser_printhex(io_read(i), 2);
                return;
            case 0:
                return;
        }
        ser_printstr("Syntax error.\r\n");
    } else {
        for (int i = 0; i < prog_size; ++i)
            io_write(i, ser_inputhex());
        _delay_ms(10);
        for (int j = 0; j < prog_size; ++j)
            ser_printhex(io_read(j), 2);
        prog_size = 0;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
