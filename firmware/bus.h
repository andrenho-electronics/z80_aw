#ifndef BUS_H_
#define BUS_H_

typedef enum { B_MC, B_Z80, B_MEMORY } BusControl;

BusControl bus_control();
void       bus_init();
void       bus_mc_takeover();
void       bus_mc_release();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
