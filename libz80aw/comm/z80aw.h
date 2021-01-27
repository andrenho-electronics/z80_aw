#ifndef Z80AW_H
#define Z80AW_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "compiler.h"

typedef struct {
    uint16_t free_memory;
} Z80AW_ControllerInfo;

typedef struct {
    uint16_t AF, BC, DE, HL, AFx, BCx, DEx, HLx, IX, IY, PC, SP;
    uint8_t R, I;
    bool HALT;
    bool valid;
} Z80AW_Registers;

typedef enum { Z80AW_NO_EVENT, Z80AW_PRINT_CHAR, Z80AW_BREAKPOINT, Z80AW_ERROR } Z80AW_EventType;

typedef enum { Z80AW_REGFETCH_DISABLED = 0, Z80AW_REGFETCH_NMI = 1, Z80AW_REGFETCH_EMULATOR = 2 } Z80AW_RegisterFetchMode;

typedef struct {
    uint8_t  char_printed;
    bool     bkp_reached;
    uint16_t pc;
} Z80AW_Event;

#define MAX_BLOCK_SIZE 512
#define UPLOAD_CHECKSUM_LOCATION 0x7ffe

int z80aw_initialize_emulator(const char* emulator_path, char* serial_port_buf, size_t serial_port_buf_sz, const char* disk_image_path);

int z80aw_init(const char* serial_port);
int z80aw_close();

bool                    z80aw_get_logging_to_stdout();
bool                    z80aw_get_assert_empty_buffer();
Z80AW_RegisterFetchMode z80aw_get_register_fetch_mode();
int                     z80aw_set_logging_to_stdout(bool v);
int                     z80aw_set_assert_empty_buffer(bool v);
int                     z80aw_set_register_fetch_mode(Z80AW_RegisterFetchMode mode);

int z80aw_set_error_callback(void (*error_cb)(const char* description, void* data), void* data);

Z80AW_ControllerInfo z80aw_controller_info();

int z80aw_write_byte(uint16_t addr, uint8_t data);
int z80aw_read_byte(uint16_t addr);

int z80aw_write_block(uint16_t addr, uint16_t sz, uint8_t const* data);
int z80aw_read_block(uint16_t addr, uint16_t sz, uint8_t* data);

uint16_t z80aw_checksum(size_t sz, uint8_t const* data);

int  z80aw_upload_compiled(DebugInformation const* di, void (*upload_callback)(void* data, float perc), void* data);
bool z80aw_is_uploaded(DebugInformation const* di);
int  z80aw_simple_compilation(const char* code, char* err_buf, size_t err_buf_sz);
DebugInformation* z80aw_simple_compilation_debug(const char* code, char* err_buf, size_t err_buf_sz);

int z80aw_cpu_reset();
int z80aw_cpu_powerdown();
int z80aw_cpu_pc();
int z80aw_cpu_step(Z80AW_Registers* reg, uint8_t* printed_char);

int z80aw_cpu_registers(Z80AW_Registers* reg);
int z80aw_cpu_nmi();

int z80aw_add_breakpoint(uint16_t addr);
int z80aw_remove_breakpoint(uint16_t addr);
int z80aw_remove_all_breakpoints();
int z80aw_query_breakpoints(uint16_t* addr, size_t addr_sz);   // return address count

int z80aw_keypress(uint8_t key);

int z80aw_cpu_next();
int z80aw_cpu_continue();
int z80aw_cpu_stop();

Z80AW_Event z80aw_last_event();

const char* z80aw_last_error();

int z80aw_finalize_emulator();

#endif
