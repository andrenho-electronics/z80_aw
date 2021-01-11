#ifndef Z80AW_H
#define Z80AW_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "compiler.h"

typedef struct {
    const char* serial_port;
    bool        log_to_stdout;
    bool        assert_empty_buffer;
} Z80AW_Config;

typedef struct {
    uint16_t free_memory;
} Z80AW_ControllerInfo;

typedef struct {
    uint16_t AF, BC, DE, HL, AFx, BCx, DEx, HLx, IX, IY, PC, SP;
    uint8_t R, I;
    bool HALT;
} Z80AW_Registers;

typedef enum { Z80AW_NO_EVENT, Z80AW_PRINT_CHAR, Z80AW_BREAKPOINT, Z80AW_ERROR } Z80AW_EventType;

typedef struct {
    Z80AW_EventType type;
    uint8_t         data;
} Z80AW_Event;

#define MAX_BLOCK_SIZE 512
#define UPLOAD_CHECKSUM_LOCATION 0x7ffe

int z80aw_initialize_emulator(const char* emulator_path, char* serial_port_buf, size_t serial_port_buf_sz, bool z80_registers);

void z80aw_init(Z80AW_Config* cfg);
void z80aw_close();

Z80AW_ControllerInfo z80aw_controller_info();

int z80aw_write_byte(uint16_t addr, uint8_t data);
int z80aw_read_byte(uint16_t addr);

int z80aw_write_block(uint16_t addr, uint16_t sz, uint8_t const* data);
int z80aw_read_block(uint16_t addr, uint16_t sz, uint8_t* data);

uint16_t z80aw_checksum(size_t sz, uint8_t const* data);

int  z80aw_upload_compiled(DebugInformation const* di, void (*upload_callback)(void* data, float perc), void* data);
bool z80aw_is_uploaded(DebugInformation const* di);
int  z80aw_simple_compilation(const char* code, char* err_buf, size_t err_buf_sz);

int z80aw_cpu_reset();
int z80aw_cpu_powerdown();
int z80aw_cpu_pc();
int z80aw_cpu_step_debug(Z80AW_Registers* reg, uint8_t* printed_char);
int z80aw_cpu_step(uint8_t* printed_char);

int z80aw_add_breakpoint(uint16_t addr);
int z80aw_remove_breakpoint(uint16_t addr);
int z80aw_remove_all_breakpoints();
int z80aw_query_breakpoints(uint16_t* addr, size_t addr_sz);   // return address count

int z80aw_keypress(uint8_t key);

int z80aw_cpu_continue();
int z80aw_cpu_stop();

Z80AW_Event z80aw_last_event();

const char* z80aw_last_error();

#endif
