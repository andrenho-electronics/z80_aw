#include "commlib.h"

#include <stdlib.h>

typedef struct CommLib {
} CommLib;

CommLib*
cl_init(const char* comfile)
{
    CommLib* cl = calloc(1, sizeof(CommLib));
    return cl;
}

void
cl_free(CommLib* cl)
{
    free(cl);
}

int
cl_enquiry()
{
    return 0;
}

// vim:ts=4:sts=4:sw=4:expandtab
