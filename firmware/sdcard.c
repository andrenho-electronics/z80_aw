#include "sdcard.h"

#include <util/delay.h>

#include "buffer.h"
#include "memory.h"

#define CMD0   0
#define CMD8   8
#define CMD17  17
#define CMD24  24
#define CMD55  55
#define CMD58  58
#define ACMD41 41

#define MAX_READ_ATTEMPTS 20
#define MAX_WRITE_ATTEMPTS 100

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

static void write_byte(uint16_t idx, uint8_t byte, void* data)
{
    buffer[idx] = byte;
    (void) data;
}

void sdcard_load_boot()
{
    sdcard_read_block(0, write_byte, NULL);
    memory_write_page(0, buffer, 512, NULL);
}

bool sdcard_read_block(uint32_t block, void(*rd)(uint16_t idx, uint8_t byte, void* data), void* data)
{
    sd_cs(true);
       
    // send read command
    sd_command(CMD17, block, 0);
    R1 r = { .value = sd_recv_spi_byte() };
    last_response = r;
    if (r.value != 0) {
        sd_cs(false);
        last_stage = SD_READ_REJECTED;
        return false;
    }

    // read data
    uint8_t rr = 0;
    for (int i = 0; i < MAX_READ_ATTEMPTS; ++i) {
        rr = sd_send_spi_byte(0xff);
        if (rr == 0xfe)
            goto read_data;
        _delay_ms(10);
    }

    // read timeout
    sd_cs(false);
    last_stage = SD_READ_TIMEOUT;
    return false;

read_data:
    for (int i = 0; i < 512; ++i)
        rd(i, sd_send_spi_byte(0xff), data);

    // crc
    sd_send_spi_byte(0xff);
    sd_send_spi_byte(0xff);

    last_stage = SD_READ_OK;
    sd_cs(false);
    return true;
}

bool sdcard_write_block(uint32_t block, uint8_t(*wd)(uint16_t idx, void* data), void* data)
{
    sd_cs(true);
       
    // send read command
    sd_command(CMD24, block, 0);
    R1 r = { .value = sd_recv_spi_byte() };
    last_response = r;
    if (r.value != 0) {
        sd_cs(false);
        last_stage = SD_WRITE_REJECTED;
        return false;
    }

    // write data to card
    sd_send_spi_byte(0xfe);
    for (uint16_t i = 0; i < 512; ++i)
        sd_send_spi_byte(wd(i, data));

    // wait for a response
    uint8_t rr = 0;
    for (int i = 0; i < MAX_WRITE_ATTEMPTS; ++i) {
        rr = sd_send_spi_byte(0xff);
        if (rr != 0xff)
            goto response_received;
        _delay_ms(10);
    }

    // response timeout
    sd_cs(false);
    last_stage = SD_WRITE_TIMEOUT;
    return false;

response_received:
    if ((rr & 0x1f) != 0x5) {
        sd_cs(false);
        last_response.value = rr;
        last_stage = SD_WRITE_DATA_REJECTED;
        return false;
    }

    // wait for write to finish
    for (int i = 0; i < MAX_WRITE_ATTEMPTS; ++i) {
        rr = sd_send_spi_byte(0xff);
        if (rr != 0x0)
            goto response_data_received;
        _delay_ms(10);
    }

    // response timeout
    sd_cs(false);
    last_stage = SD_WRITE_DATA_TIMEOUT;
    return false;

response_data_received:
    last_stage = SD_WRITE_OK;
    sd_cs(false);
    return true;
}
