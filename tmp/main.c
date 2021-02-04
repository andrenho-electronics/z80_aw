#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "serial.h"
#include "sdcard.h"

static void print_r1(R1 r1)
{
    printf("%02X ", r1);
    if (*(uint8_t*)&r1 == 0) {
        printf_P("general error ");
        return;
    }
    if (r1.idle)
        printf_P("idle ");
    if (r1.erase_reset)
        printf_P("erase_reset ");
    if (r1.illegal_cmd)
        printf_P("illegal_cmd ");
    if (r1.crc_error)
        printf_P("crc_error ");
    if (r1.address_err)
        printf_P("address_err ");
    if (r1.param_err)
        printf_P("param_err ");
    if (*(uint8_t*)&r1 == 0)
        printf_P("ok ");
}

int main()
{
    serial_init();

    printf_P(PSTR("\e[1;1H\e[2J"));  // clear screen

    printf_P(PSTR("SD card setup... "));
    sdcard_setup();
    printf_P(PSTR("done.\r\n"));

    printf_P(PSTR("\n\rSD card initialization..."));
    sdcard_initialize();
    printf_P(PSTR("done.\r\n"));

    printf_P(PSTR("\r\nSD card set SPI mode (expected 1): "));
    print_r1(sdcard_set_spi_mode());
    printf_P(PSTR(".\n\r"));

    /*
    r = 0xff;
    int i = 0;
    while (r != 0) {
        printf_P(PSTR("\r\nSD card initialization process: "));
        r = sdcard_init_process();
        printf_P(PSTR("%d\n\r"), r);
        _delay_ms(200);
        if (i++ == 8) {
            printf("Could not initialize card.\r\n");
            break;
        }
    }
    */

    for (;;);
}
