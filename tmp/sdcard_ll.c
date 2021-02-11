#include "sdcard_ll.h"

#define LOG

#ifdef LOG
#  include <stdio.h>
#  include <avr/pgmspace.h>
#endif
#include <avr/io.h>
#include <util/delay.h>

#define CS   PIND6
#define MOSI PINB5
#define MISO PINB6
#define SCK  PINB7

void sd_setup()
{
    // output pins
    DDRB |= (1 << MOSI) | (1 << SCK);
    DDRD |= (1 << CS);

    // input pull up on MISO
    DDRB |= (1 << MISO);

    // enable SPI, set as MASTER, clock to fosc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

void sd_cs(bool enabled)
{
    sd_send_spi_byte(0xff);
    if (enabled) {
#ifdef LOG
        printf_P(PSTR("\e[0;32m^\e[0m"));
#endif
        PORTD &= ~(1 << CS);
    } else {
#ifdef LOG
        printf_P(PSTR("\e[0;32m_\e[0m"));
#endif
        PORTD |= (1 << CS);
    }
    sd_send_spi_byte(0xff);
}

uint8_t sd_send_spi_byte(uint8_t byte)
{
#ifdef LOG
    if (byte == 0xff)
        printf_P(PSTR("\e[0;32m.\e[0m"));
    else
        printf_P(PSTR("\e[0;32m%02X\e[0m "), byte);
#endif
    SPDR = byte;
    while (!(SPSR & (1 << SPIF)));
    uint8_t r = SPDR;
#ifdef LOG
    if (r == 0xff)
        printf_P(PSTR("\e[0;31m.\e[0m"));
    else
        printf_P(PSTR("\e[0;31m%02X\e[0m "), r);
#endif
    return r;
}

uint8_t sd_recv_spi_byte()
{
    uint8_t i = 0, r;
    while ((r = sd_send_spi_byte(0xff)) == 0xff) {
        ++i;
        if (i > 8)
            break;  // timeout
    }
    return r;
}

void sd_command(uint8_t cmd, uint32_t args, uint8_t crc)
{
    sd_send_spi_byte(cmd | 0x40);
    sd_send_spi_byte((uint8_t)(args >> 24));
    sd_send_spi_byte((uint8_t)(args >> 16));
    sd_send_spi_byte((uint8_t)(args >> 8));
    sd_send_spi_byte((uint8_t)args);
    sd_send_spi_byte(crc | 0x1);
}

