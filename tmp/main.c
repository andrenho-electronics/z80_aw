#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "serial.h"
#include "sdcard.h"

static void print_r1(R1 r1)
{
    uint8_t value = *(uint8_t*)&r1;
    printf("(%02X) ", value);
    if (value == 0xff) {
        printf_P(PSTR("general error "));
        return;
    }
    if (r1.idle)
        printf_P(PSTR("idle "));
    if (r1.erase_reset)
        printf_P(PSTR("erase_reset_err "));
    if (r1.illegal_cmd)
        printf_P(PSTR("illegal_cmd "));
    if (r1.crc_error)
        printf_P(PSTR("crc_error "));
    if (r1.erase_seq)
        printf_P(PSTR("erase_seq_err "));
    if (r1.address_err)
        printf_P(PSTR("address_err "));
    if (r1.param_err)
        printf_P(PSTR("param_err "));
    if (value == 0)
        printf_P(PSTR("ok "));
    printf("\b");
}

static void print_r7(R7 r7)
{
    print_r1(r7.r1);
    printf_P(PSTR(", "));
    printf_P(PSTR("(%lX) "), r7.ocr);
#define BIT(n, text) if (r7.ocr & ((uint32_t) 1 << n)) printf_P(PSTR(text));
    BIT(31, "card_busy ");
    BIT(30, "card_capacity_status ");
    BIT(29, "uhs_ii_card_status ");
    BIT(24, "can_switch_to_1.8v ");
    BIT(23, "3.5_3.6v ");
    BIT(22, "3.4_3.5v ");
    BIT(21, "3.3_3.4v ");
    BIT(19, "3.2_3.3v ");
    BIT(18, "3.1_3.2v ");
    BIT(17, "3.0_3.1v ");
    BIT(16, "2.9_3.0v ");
    BIT(15, "2.8_2.9v ");
#undef BIT
}

int main()
{
    serial_init();

    printf_P(PSTR("\e[1;1H\e[2J"));  // clear screen

    printf_P(PSTR("SD card setup... "));
    sdcard_setup();
    printf_P(PSTR("done.\r\n"));

    printf_P(PSTR("SD card initialization..."));
    sdcard_initialize();
    printf_P(PSTR("done.\r\n"));

    printf_P(PSTR("SD card set SPI mode (expected 1): "));
    print_r1(sdcard_set_spi_mode());
    printf_P(PSTR(".\n\r"));

    printf_P(PSTR("SD card interface condition: "));
    print_r7(sdcard_if_cond());
    printf_P(PSTR(".\n\r"));

    printf_P(PSTR("SD card initialization process: \r\n"));
    for (int i = 0; i < 8; ++i) {
        printf_P(PSTR("Attempt %d/8: "), i+1);
        R1 r1 = sdcard_init_process();
        print_r1(r1);
        printf_P(PSTR(".\n\r"));
        uint8_t value = *(uint8_t*)&r1;
        if (value == 0)
            goto done;
        _delay_ms(200);
    }

    printf_P(PSTR("Error initializing card.\n\r"));
    for (;;);
done:
    printf_P(PSTR("Card initialized.\n\r"));
    for (;;);
}
