#include "comm.h"
#include "io.h"
#include "serial.h"

int main()
{
    serial_init();
    io_init();

    while (1)
        comm_listen();
}

// vim:ts=4:sts=4:sw=4:expandtab
