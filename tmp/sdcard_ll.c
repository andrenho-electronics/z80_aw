#include "sdcard_ll.h"

#ifdef LOG
#  include <stdio.h>
#  include <avr/pgmspace.h>
#endif
#include <avr/io.h>
#include <util/delay.h>

#define CS   PINB4
#define MOSI PINB5
#define MISO PINB6
#define SCK  PINB7

#define MAX_READ_ATTEMPTS 20
#define MAX_WRITE_ATTEMPTS 100

void sd_setup()
{
    // output pins
    DDRB |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    // input pull up on MISO
    DDRB |= (1 << MISO);

    // enable SPI, set as MASTER, clock to fosc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

void sd_cs(bool enabled)
{
    if (enabled) {
#ifdef LOG
        printf_P(PSTR("\e[0;32m^\e[0m"));
#endif
        PORTB &= ~(1 << CS);
    } else {
#ifdef LOG
        printf_P(PSTR("\e[0;32m_\e[0m"));
#endif
        PORTB |= (1 << CS);
    }
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

R1 sd_command_r1(uint8_t cmd, uint32_t args, uint8_t crc)
{
    // enable card
    sd_send_spi_byte(0xff);
    sd_cs(true);
    sd_send_spi_byte(0xff);

    // send command
    sd_send_spi_byte(cmd | 0x40);
    sd_send_spi_byte((uint8_t)(args >> 24));
    sd_send_spi_byte((uint8_t)(args >> 16));
    sd_send_spi_byte((uint8_t)(args >> 8));
    sd_send_spi_byte((uint8_t)args);
    sd_send_spi_byte(crc | 0x1);

    // read response
    R1 r = { .value = sd_recv_spi_byte() };

    // disable card
    sd_send_spi_byte(0xff);
    sd_cs(false);
    sd_send_spi_byte(0xff);

    return r;
}

R3 sd_command_r3(uint8_t cmd, uint32_t args, uint8_t crc)
{
    // enable card
    sd_send_spi_byte(0xff);
    sd_cs(true);
    sd_send_spi_byte(0xff);

    // send command
    sd_send_spi_byte(cmd | 0x40);
    sd_send_spi_byte((uint8_t)(args >> 24));
    sd_send_spi_byte((uint8_t)(args >> 16));
    sd_send_spi_byte((uint8_t)(args >> 8));
    sd_send_spi_byte((uint8_t)args);
    sd_send_spi_byte(crc | 0x1);

    // read response
    R3 r3 = { .value = 0 };
    r3.bytes[0] = sd_recv_spi_byte();
    if (r3.bytes[0] > 1) {
        return r3;
    }

    r3.bytes[1] = sd_send_spi_byte(0xff);
    r3.bytes[2] = sd_send_spi_byte(0xff);
    r3.bytes[3] = sd_send_spi_byte(0xff);
    r3.bytes[4] = sd_send_spi_byte(0xff);

    // disable card
    sd_send_spi_byte(0xff);
    sd_cs(false);
    sd_send_spi_byte(0xff);

    return r3;
}

R7 sd_command_r7(uint8_t cmd, uint32_t args, uint8_t crc)
{
    // enable card
    sd_send_spi_byte(0xff);
    sd_cs(true);
    sd_send_spi_byte(0xff);

    // send command
    sd_send_spi_byte(cmd | 0x40);
    sd_send_spi_byte((uint8_t)(args >> 24));
    sd_send_spi_byte((uint8_t)(args >> 16));
    sd_send_spi_byte((uint8_t)(args >> 8));
    sd_send_spi_byte((uint8_t)args);
    sd_send_spi_byte(crc | 0x1);

    // read response
    R7 r7 = { .value = 0 };
    r7.bytes[0] = sd_recv_spi_byte();
    if (r7.bytes[0] > 1) {
        return r7;
    }

    r7.bytes[1] = sd_send_spi_byte(0xff);
    r7.bytes[2] = sd_send_spi_byte(0xff);
    r7.bytes[3] = sd_send_spi_byte(0xff);
    r7.bytes[4] = sd_send_spi_byte(0xff);

    // disable card
    sd_send_spi_byte(0xff);
    sd_cs(false);
    sd_send_spi_byte(0xff);

    return r7;
}

R1 sd_command_read_block(uint8_t cmd, uint32_t block, uint8_t* data)
{
    // enable card
    sd_send_spi_byte(0xff);
    sd_cs(true);
    sd_send_spi_byte(0xff);

    // send command
    sd_send_spi_byte(cmd | 0x40);
    sd_send_spi_byte((uint8_t)(block >> 24));
    sd_send_spi_byte((uint8_t)(block >> 16));
    sd_send_spi_byte((uint8_t)(block >> 8));
    sd_send_spi_byte((uint8_t)block);
    sd_send_spi_byte(0x1);

    // read response
    R1 r = { .value = sd_recv_spi_byte() };
    if (r.value != 0xff) {
        uint8_t rr = 0;
        for (int i = 0; i < MAX_READ_ATTEMPTS; ++i) {
            rr = sd_send_spi_byte(0xff);
            if (rr == 0xfe)
                goto received;
            _delay_ms(10);
        }
        return (R1) { .value = rr };
    } else {
        return r;
    }

received:
    for (int i = 0; i < 512; ++i)
        *data++ = sd_send_spi_byte(0xff);

    // crc
    sd_send_spi_byte(0xff);
    sd_send_spi_byte(0xff);

    // disable card
    sd_send_spi_byte(0xff);
    sd_cs(false);
    sd_send_spi_byte(0xff);

    return r;
}

R1 sd_command_write_block(uint8_t cmd, uint32_t block, uint8_t* data)
{
    // enable card
    sd_send_spi_byte(0xff);
    sd_cs(true);
    sd_send_spi_byte(0xff);

    // send command
    sd_send_spi_byte(cmd | 0x40);
    sd_send_spi_byte((uint8_t)(block >> 24));
    sd_send_spi_byte((uint8_t)(block >> 16));
    sd_send_spi_byte((uint8_t)(block >> 8));
    sd_send_spi_byte((uint8_t)block);
    sd_send_spi_byte(0x1);

    // read response
    R1 r = { .value = sd_recv_spi_byte() };
    if (r.value == 0x0) {
        spi_send_spi_byte(0xfe);  // start block
        for (int i = 0; i < 512; ++i)
            spi_send_spi_byte(data[i]);
        uint8_t rr;
        for (int i = 0; i < MAX_WRITE_ATTEMPTS; ++i)
            if ((rr = spi_send_spi_byte(0xff)) != 0xff)
                goto ok;
        return { .value = 0xff };
ok:
        if (rr & 0x1f == 0x05) {  // data accepted
            // wait for write to finish           
            
        }
    }

    // disable card
    sd_send_spi_byte(0xff);
    sd_cs(false);
    sd_send_spi_byte(0xff);

    return r;
}
