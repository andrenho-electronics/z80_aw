#ifndef BREAKPOINTS_H_
#define BREAKPOINTS_H_

#include <stdbool.h>
#include <stdint.h>

#define MAX_BREAKPOINTS 16

bool bkp_add(uint16_t bkp);
void bkp_remove(uint16_t bkp);
int  bkp_query(uint16_t bkps[16]);
bool bkp_in_list(uint16_t addr);
void bkp_remove_all();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
