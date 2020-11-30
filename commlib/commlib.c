#include "commlib.h"

#include <stdlib.h>
#include <string.h>

#define RED   \u001b[31m
#define GREEN \u001b[32m
#define RESET \u001b[0m

typedef struct CommLib {
    int last_error;
} CommLib;

CommLib*
cl_init(const char* comfile, int speed)
{
    CommLib* cl = calloc(1, sizeof(CommLib));
    cl->last_error = 0;
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
    if (code == 0)
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
