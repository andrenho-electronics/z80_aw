#include "commlib.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "messages.h"
#include "serial.h"

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define MAGENTA "\033[35m"
#define RESET   "\033[0m"

typedef struct CommLib {
    int  fd;
    int  last_error;
    bool z80_initialized;
} CommLib;


CommLib*
cl_init(const char* comfile, int speed)
{
    CommLib* cl = calloc(1, sizeof(CommLib));
    cl->last_error = 0;
    cl->fd = serial_open(comfile, speed);
    cl->z80_initialized = false;
    return cl;
}

void
cl_free(CommLib* cl)
{
    free(cl);
}

char*
cl_strerror(int code)
{
    if (code >= 0)
        return strerror(code);

    switch (code) {
        case NAK:
            return "Not acknowledged.";
        case BUS_BUSY:
            return "This operation cannot be completed because the bus is currently busy.";
        case INVALID_COMMAND:
            return "Command not implemented in controller.";
        case Z80_NOT_INITIALIZED:
            return "The Z80 was not yet initialized.";
        default:
            return "Unknown error.";
    }
}

void
cl_perror(CommLib* cl)
{
    if (cl->last_error == 0)
        printf("No error.\n");
    else {
        printf(RED);
        printf(cl_strerror(cl->last_error));
        printf(RESET);
        printf("\n");
    }
}

void
cl_set_debug(CommLib* cl, bool v)
{
    (void) cl;
    serial_debug = v;
}

int
cl_enquiry(CommLib* cl)
{
    serial_send(cl->fd, CMD_ENQUIRY);
    cl->last_error = serial_recv(cl->fd);
    return cl->last_error;
}

int
cl_read_memory(CommLib* cl, uint16_t addr, uint8_t* buf, size_t sz)
{
    serial_send(cl->fd, CMD_READ);
    serial_send16(cl->fd, addr);
    serial_send16(cl->fd, sz);
    cl->last_error = 0;
    for (size_t i = 0; i < sz; ++i) {
        int r = serial_recv(cl->fd);
        if (r < 0 || r > 255) {
            cl->last_error = r;
            break;
        }
        buf[i] = (uint8_t) r;
    }
    return cl->last_error;
}

int
cl_write_memory(CommLib* cl, uint16_t addr, uint8_t const* data, size_t sz)
{
    serial_send(cl->fd, CMD_WRITE);
    serial_send16(cl->fd, addr);
    serial_send16(cl->fd, sz);
    for (size_t i = 0; i < sz; ++i)
        serial_send(cl->fd, data[i]);

    cl->last_error = serial_recv(cl->fd);
    return cl->last_error;
}

int
cl_status(CommLib* cl, CL_Status* status)
{
    cl->last_error = 0;
    serial_send(cl->fd, CMD_STATUS);
    status->cycle = serial_recv16(cl->fd);
    status->addr = serial_recv16(cl->fd);
    status->data = serial_recv(cl->fd);
    uint8_t in = serial_recv(cl->fd);
    status->inputs = *(Inputs*)(&in);
    return cl->last_error;
}

int
cl_cycle(CommLib* cl)
{
    if (!cl->z80_initialized) {
        cl->last_error = Z80_NOT_INITIALIZED;
    } else {
        serial_send(cl->fd, CMD_CYCLE);
        cl->last_error = serial_recv(cl->fd);
    }
    return cl->last_error;
}

int
cl_init_z80(CommLib* cl)
{
    serial_send(cl->fd, CMD_INIT);
    cl->last_error = serial_recv(cl->fd);
    if (cl->last_error == ACK)
        cl->z80_initialized = true;
    return cl->last_error;
}

int
cl_reset_z80(CommLib* cl)
{
    serial_send(cl->fd, CMD_RESET);
    cl->last_error = serial_recv(cl->fd);
    if (cl->last_error == ACK)
        cl->z80_initialized = false;
    return cl->last_error;
}

// vim:ts=4:sts=4:sw=4:expandtab
