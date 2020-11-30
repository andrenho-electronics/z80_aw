#include "commlib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "messages.h"
#include "serial.h"

#define RED   "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

typedef struct CommLib {
    int fd;
    int last_error;
} CommLib;


CommLib*
cl_init(const char* comfile, int speed)
{
    CommLib* cl = calloc(1, sizeof(CommLib));
    cl->last_error = 0;
    cl->fd = serial_open(comfile, speed);
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
    }
}

int
cl_enquiry(CommLib* cl)
{
    serial_send(cl->fd, ENQUIRY);
    if (serial_recv(cl->fd) == ACK)
        return 0;
    else
        return NAK;
}

// vim:ts=4:sts=4:sw=4:expandtab
