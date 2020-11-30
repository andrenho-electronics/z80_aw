#include "commlib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "messages.h"

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
    cl->fd = open_serial(comfile, speed);
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
    return 0;
}

// vim:ts=4:sts=4:sw=4:expandtab
