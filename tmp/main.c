#include <stdio.h>
#include <avr/pgmspace.h>

#include "serial.h"
#include "sdcard.h"

int main()
{
    serial_init();

    printf_P(PSTR("\e[1;1H\e[2J"));  // clear screen

    printf_P(PSTR("SD card setup...\n\r"));
    sdcard_setup();

    printf_P(PSTR("\n\rSD card initialization...\n\r"));
    sdcard_initialize();

    printf_P(PSTR("\r\nSD card set SPI mode (expected 1): "));
    uint8_t r = sdcard_set_spi_mode();
    printf_P(PSTR("%d\n\r"), r);

    for (;;);
}
