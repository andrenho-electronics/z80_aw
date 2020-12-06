#include "bus.h"

#include "lowlevel.h"

static BusControl control;

BusControl bus_control()
{
    return control;
}

void bus_init()
{
}

void bus_mc_takeover()
{
}

void bus_mc_release()
{
}

// vim:ts=4:sts=4:sw=4:expandtab
