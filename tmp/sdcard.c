#include "sdcard.h"

#define LOG_OUTPUT 1

#include <avr/io.h>
#include <util/delay.h>
#ifdef LOG_OUTPUT
#  include <stdio.h>
#endif
#include <avr/pgmspace.h>

#define CS   PINB4
#define MOSI PINB5
#define MISO PINB6
#define SCK  PINB7

#define CS_ENABLE()   { PORTB &= ~(1 << CS); }
#define CS_DISABLE()  { PORTB |= (1 << CS); }

void sdcard_setup()
{
    // output pins
    DDRB |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    // input pull up on MISO
    DDRB |= (1 << MISO);

    // enable SPI, set as MASTER, clock to fosc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

static uint8_t spi_send(uint8_t data)
{
#ifdef LOG_OUTPUT
    if (data == 0xff)
        printf_P(PSTR("\e[0;32m.\e[0m"));
    else
        printf_P(PSTR("\e[0;32m%02X\e[0m "), data);
#endif
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    uint8_t r = SPDR;
#ifdef LOG_OUTPUT
    if (r == 0xff)
        printf_P(PSTR("\e[0;31m.\e[0m"));
    else
        printf_P(PSTR("\e[0;31m%02X\e[0m "), r);
#endif
    return r;
}

static uint8_t sd_recv()
{
    uint8_t i = 0, r;
    while ((r = spi_send(0xff)) == 0xff) {
        ++i;
        if (i > 8)
            break;  // timeout
    }
    return r;
}

static uint8_t sd_send(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    // enable card
    spi_send(0xff);
    CS_ENABLE();
    spi_send(0xff);

    // send command
    spi_send(cmd | 0x40);
    spi_send((uint8_t)(arg >> 24));
    spi_send((uint8_t)(arg >> 16));
    spi_send((uint8_t)(arg >> 8));
    spi_send((uint8_t)arg);
    spi_send(crc | 0x1);

    // read response
    uint8_t r = sd_recv();

    // disable card
    spi_send(0xff);
    CS_DISABLE();
    spi_send(0xff);

    return r;
}


static uint8_t sd_send_app(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t r = sd_send(55, 0, 0);  // CMD55
    return sd_send(cmd, arg, crc);
}

void sdcard_initialize()
{
    // power up card
    CS_DISABLE();
    _delay_ms(1);
    for (uint8_t i = 0; i < 10; ++i)
        spi_send(0xff);
    
    // deselect SD card
    CS_DISABLE();
    spi_send(0xff);
}

uint8_t sdcard_set_spi_mode()
{
    return sd_send(0, 0, 0x94);  // CMD0
}

uint8_t sdcard_init_process()
{
    return sd_send_app(41, 0x40000000, 0);
}
