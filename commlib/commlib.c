#include "commlib.h"

#include <stdlib.h>
#include <string.h>

typedef struct CommLib {
} CommLib;

CommLib*
cl_init(const char* comfile, int speed)
{
    CommLib* cl = calloc(1, sizeof(CommLib));
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
        default:
            return "Unknown error.";
    }
}

int
cl_enquiry(CommLib* cl)
{
    return 0;
}

// vim:ts=4:sts=4:sw=4:expandtab
