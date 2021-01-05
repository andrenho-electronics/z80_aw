#include "z80aw.h"

#include <stdarg.h>
#include <stdio.h>
#include <protocol.h>

#include "comm.h"
#include "z80aw_priv.h"

static char last_error[256] = "No error.";
static bool log_to_stdout = false;

void z80aw_init(Z80AW_Config* cfg)
{
    open_serial_port(cfg->serial_port, cfg->log_to_stdout);
    log_to_stdout = cfg->log_to_stdout;
}

void z80aw_close()
{
    close_serial_port();
}

void z80aw_set_error(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(last_error, sizeof last_error, fmt, ap);
    if (log_to_stdout)
        printf("\e[0;31mERROR: %s\e[0m\n", last_error);
    va_end(ap);
}

const char* z80aw_last_error()
{
    return last_error;
}

Z80AW_ControllerInfo z80aw_controller_info()
{
    Z80AW_ControllerInfo c;
    zsend_noreply(Z_CTRL_INFO);
    c.free_memory = zrecv16();
    z_assert_empty_buffer();
    return c;
}

int z80aw_write_byte(uint16_t addr, uint8_t data)
{
    int r = z80aw_write_block(addr, 1, &data);
    if (r < 0)
        return r;
    return 0;
}

int z80aw_read_byte(uint16_t addr)
{
    uint8_t c;
    int r = z80aw_read_block(addr, 1, &c);
    if (r < 0)
        return r;
    return c;
}

int z80aw_write_block(uint16_t addr, uint16_t sz, uint8_t const* data)
{
    uint16_t checksum = z80aw_checksum(sz, data);
    zsend_noreply(Z_WRITE_BLOCK);
    zsend_noreply(addr & 0xff);
    zsend_noreply(addr >> 8);
    zsend_noreply(sz & 0xff);
    zsend_noreply(sz >> 8);
    for (uint16_t i = 0; i < sz; ++i)
        zsend_noreply(data[i]);
    uint16_t rchecksum = zrecv16();
    if (checksum != rchecksum)
        ERROR("When writing to memory, received checksum (0x%x) does not match with calculated checksum (0x%x).", rchecksum, checksum);
    z_assert_empty_buffer();
    return 0;
}

int z80aw_read_block(uint16_t addr, uint16_t sz, uint8_t* data)
{
    zsend_noreply(Z_READ_BLOCK);
    zsend_noreply(addr & 0xff);
    zsend_noreply(addr >> 8);
    zsend_noreply(sz & 0xff);
    zsend_noreply(sz >> 8);
    for (uint16_t i = 0; i < sz; ++i)
        data[i] = zrecv();
    z_assert_empty_buffer();
    return 0;
}

uint16_t z80aw_checksum(size_t sz, uint8_t const* data)
{
    uint16_t checksum1 = 0, checksum2 = 0;
    for (size_t i = 0; i < sz; ++i) {
        checksum1 = (checksum1 + data[i]) % 255;
        checksum2 = (checksum2 + checksum1) % 255;
    }
    return checksum1 | (checksum2 << 8);
}

int z80aw_upload_compiled(DebugInformation const* di)
{
    // write data
    size_t i = 0;
    for (Binary const* bin = debug_binary(di, i); bin; bin = debug_binary(di, ++i)) {
        int r = z80aw_write_block(bin->addr, bin->sz, bin->data);
        if (r != 0)
            return r;
    }
    
    // write checksum
    uint16_t chk = debug_binary_checksum(di);
    uint8_t data[] = { chk & 0xff, chk >> 8 };
    int r = z80aw_write_block(UPLOAD_CHECKSUM_LOCATION, 2, data);
    if (r != 0)
        return r;
    
    return 0;
}

bool z80aw_is_uploaded(DebugInformation const* di)
{
    uint8_t data[2];
    z80aw_read_block(UPLOAD_CHECKSUM_LOCATION, 2, data);
    uint16_t chk = debug_binary_checksum(di);
    
    return (data[0] == (chk & 0xff)) && (data[1] == (chk >> 8));
}