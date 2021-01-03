#include "z80.h"

#include "bus.h"
#include "lowlevel.h"
#include "memory.h"
#include "serial.h"
#include "repl.h"

#include "wait.h"

Status   z80_last_status;
uint16_t z80_last_pc = 0;

static uint8_t  last_key_pressed = 0;
static uint16_t z80_cycle_number = 0;

static void z80_clock();

void z80_update_status()
{
#define Z z80_last_status
    bus_mc_release();
    Z.cycle = z80_cycle_number;
    Z.addr_bus = -1;
    Z.data_bus = -1;
    Z.m1 = get_M1();
    Z.iorq = get_IORQ();
    Z.halt = get_HALT();
    Z.busack = get_BUSACK();
    Z.wait = get_WAIT();
    Z.int_ = get_INT();
    Z.nmi = get_NMI();
    Z.reset = get_ZRST();
    Z.busreq = get_BUSREQ();
    Z.mreq = get_MREQ();
    Z.rd = get_RD();
    Z.wr = get_WR();
    if (Z.rd == 0 || Z.wr == 0) {
        Z.addr_bus = memory_read_addr();
        Z.data_bus = memory_read_data();
        if (Z.m1 == 0)
            z80_last_pc = Z.addr_bus;
    }
#undef Z 
}

bool z80_controls_bus()
{
    if (z80_last_status.busack == 0 || z80_last_status.reset == 0)
        return false;
    return true;
}

static void z80_iorq_requested()
{
    uint16_t addr = memory_read_addr();
    if ((addr & 0xff) == 0x00) {   // video device
        uint8_t data = addr >> 8;
        serial_printstr(PSTR("Char "));
        serial_printhex8(data);
        if (data >= 32 && data < 127) {
            serial_send(' ');
            serial_send('\'');
            serial_send(data);
            serial_send('\'');
        }
        serial_printstr(PSTR(" was sent to the display.\r\n"));
    } else if ((addr & 0xff) == 0x01) {  // last keyboard press
        serial_printstr(PSTR("Last pressed key requested ("));
        serial_printhex8(last_key_pressed);
        serial_printstr(PSTR(").\r\n"));
        memory_set_data(last_key_pressed);
        z80_clock();
        z80_last_status.data_bus = last_key_pressed;
    } else {
        serial_printstr(PSTR("IORQ requested by device "));
        serial_printhex8(addr & 0xff);
        serial_puts();
    }
}

static void z80_clock()
{
    set_ZCLK(1);
    wait();
    set_ZCLK(0);
    wait();
    bool last_iorq = z80_last_status.iorq;
    z80_update_status();
    if (last_iorq == 1 && get_IORQ() == 0) {
        z80_iorq_requested();
    }
    if (z80_last_status.reset == 1)
        ++z80_cycle_number;
}

void z80_clock_cycle(bool request_bus)
{
    bus_mc_release();
    set_BUSREQ(!request_bus);
    z80_clock();
}

void z80_powerdown()
{
    set_ZRST(0);
    z80_cycle_number = 0;
    z80_clock_cycle(false);
}

void z80_init()
{
    bus_mc_release();

    set_ZRST(0);
    z80_cycle_number = 0;
    wait();

    for (int i = 0; i < 50; ++i)
        z80_clock_cycle(false);

    set_ZRST(1);
    wait();
    z80_clock_cycle(false);
}

void z80_keypress(uint8_t key)
{
    last_key_pressed = key;

    bus_mc_release();

    // fire interrupt
    set_INT(0);
    set_BUSREQ(1);
    for (int i = 0; i < 15; ++i) {
        z80_clock();
        repl_status();
        if (get_IORQ() == 0)
            goto z80_response;
    }
    set_INT(1);   // a interrupt request was not accepted by Z80
    return;

z80_response:
    set_INT(1);

    serial_send('-');
    serial_puts();

    do {
        memory_set_data(0xcf);
        z80_clock();
        z80_last_status.data_bus = 0xcf;
        repl_status();
    } while (get_IORQ() == 0);
}

uint16_t z80_step()
{
    bool busack = 1, m1 = 1;

    // run cycle until M1
    while (m1 == 1) {
        bus_mc_release();
        set_BUSREQ(true);
        set_ZCLK(1);
        wait();
        set_ZCLK(0);
        wait();
        z80_update_status();
        // z80_clock_cycle(false);
        m1 = get_M1();
    }

    // update PC
    bus_mc_release();
    uint16_t pc = memory_read_addr();

    // run cycle until BUSACK
    while (busack == 1) {
        bus_mc_release();
        set_BUSREQ(false);
        set_ZCLK(1);
        wait();
        set_ZCLK(0);
        wait();
        z80_update_status();
        // z80_clock_cycle(true);
        // TODO - check interrupts
        busack = get_BUSACK();
    }

    return pc;
}

// vim:ts=4:sts=4:sw=4:expandtab
