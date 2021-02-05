#include "sdcard.h"

#include <util/delay.h>

#define CMD0   0
#define CMD8   8
#define CMD17  17
#define CMD24  24
#define CMD55  55
#define CMD58  58
#define ACMD41 41

static SDCardStage last_stage = SD_NOT_INITIALIZED;
static R1          last_response = { .value = 0xff };

void sdcard_set_last_status(SDCardStage stage, R1 r1)
{
    last_stage = stage;
    last_response = r1;
}

SDCardStage sdcard_last_stage()
{
    return last_stage;
}

R1 sdcard_last_response()
{
    return last_response;
}

static void sdcard_initialize()
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

static R1 sdcard_go_idle()
{
    sd_cs(true);
    sd_command(CMD0, 0, 0x94);
    R1 r1 = { .value = sd_recv_spi_byte() };
    sd_cs(false);
    return r1;
}

static R1 sdcard_if_cond(uint32_t* response)
{
    sd_cs(true);
    sd_command(CMD8, 0x1AA, 0x86);
    R1 r1 = { .value = sd_recv_spi_byte() };
    if (r1.value <= 1) {
        *response = 0;
        *response |= (uint32_t) sd_send_spi_byte(0xff) << 24;
        *response |= (uint32_t) sd_send_spi_byte(0xff) << 16;
        *response |= (uint32_t) sd_send_spi_byte(0xff) << 8;
        *response |= (uint32_t) sd_send_spi_byte(0xff);
    }
    sd_cs(false);
    return r1;
}

static R1 sdcard_init_process(uint32_t* response)
{
    sd_cs(true);
    sd_command(CMD55, 0, 0);
    R1 r1 = { .value = sd_recv_spi_byte() };
    sd_cs(false);
    if (r1.value > 1)
        return r1;

    sd_cs(true);
    sd_command(ACMD41, 0x40000000, 0);
    r1 = (R1) { .value = sd_recv_spi_byte() };
    if (r1.value <= 1) {
        *response = 0;
        *response |= (uint32_t) sd_send_spi_byte(0xff) << 24;
        *response |= (uint32_t) sd_send_spi_byte(0xff) << 16;
        *response |= (uint32_t) sd_send_spi_byte(0xff) << 8;
        *response |= (uint32_t) sd_send_spi_byte(0xff);
    }
    sd_cs(false);

    return r1;
}

bool sdcard_init()
{
    sdcard_set_last_status(SD_MCU_SETUP, (R1) { .value = 0x0 });
    sd_setup();

    last_stage = SD_RESET;
    sdcard_initialize();

    last_stage = SD_GO_IDLE;
    last_response = sdcard_go_idle();
    if (last_response.value != 0x1)
        return false;

    uint32_t response = 0;
    last_stage = SD_IF_COND;
    last_response = sdcard_if_cond(&response);
    if (last_response.value > 1)
        return false;

    last_stage = SD_INIT;
    for (int i = 0; i < 16; ++i) {
        last_response = sdcard_init_process(&response);
        if (last_response.value == 0)
            return true;
        _delay_ms(50);
    }
    return false;
}

/*
void sdcard_setup()
{
    sd_setup();
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

R3 sdcard_get_info()
{
    return sd_command_r3(CMD58, 0, 0);
}

R1 sdcard_read_block(uint32_t block, uint8_t* data)
{
    return sd_command_read_block(CMD17, block, data);
}

R1 sdcard_write_block(uint32_t block, uint8_t* data)
{
    return sd_command_write_block(CMD24, block, data);
}
*/
