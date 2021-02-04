#include "sdcard.h"

#include <util/delay.h>

#define CMD0   0
#define CMD8   8
#define CMD55  55
#define ACMD41 41

void sdcard_setup()
{
    sd_setup();
}

void sdcard_initialize()
{
    // power up card
    sd_cs(false);
    _delay_ms(1);
    for (uint8_t i = 0; i < 10; ++i)
        sd_send_spi_byte(0xff);
    
    // deselect SD card
    sd_cs(false);
    sd_send_spi_byte(0xff);
}

R1 sdcard_set_spi_mode()
{
    return sd_command_r1(CMD0, 0, 0x94);
}

R7 sdcard_if_cond()
{
    return sd_command_r7(CMD8, 0x0000001AA, 0x86);
}

R1 sdcard_init_process()
{
    R1 r = sd_command_r1(CMD55, 0, 0);
    uint8_t value = *(uint8_t*)&r;
    if (value > 1)
        return r;

    return sd_command_r1(ACMD41, 0x40000000, 0);
}
