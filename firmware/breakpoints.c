#include "breakpoints.h"

#include <stddef.h>

uint16_t breakpoints[MAX_BREAKPOINTS] = { 0 };

bool bkp_add(uint16_t bkp)
{
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i) {
        if (breakpoints[i] == bkp)  // avoid duplicates
            return true;
        if (breakpoints[i] == 0) {
            breakpoints[i] = bkp;
            return true;
        }
    }
    return false;
}

void bkp_remove(uint16_t bkp)
{
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i)
        if (breakpoints[i] == bkp)
            breakpoints[i] = 0;
}

int bkp_query(uint16_t bkps[16])
{
    size_t j = 0;
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i) {
        if (breakpoints[i] != 0)
            bkps[j++] = breakpoints[i];
    }
    return j;
}

void bkp_remove_all()
{
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i)
        breakpoints[i] = 0;
}

bool bkp_in_list(uint16_t addr)
{
    if (addr == 0)
        return false;
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i)
        if (breakpoints[i] == addr)
            return true;
    return false;
}

// vim:ts=4:sts=4:sw=4:expandtab
