#include "z80aw.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "protocol.h"
#include "comm.h"
#include "z80aw_priv.h"
#include "logging.h"

static char last_error[256] = "No error.";
static bool wait_for_emulator = true;
static bool z80_power = false;
static void (*error_cb)(const char* description, void* data) = NULL;
static void* error_data = NULL;
static bool regfetch_enabled = false;
static Z80AW_RegisterFetchMode register_fetch_mode = Z80AW_REGFETCH_DISABLED;

int z80aw_init(const char* serial_port)
{
    if (open_serial_port(serial_port) != 0)
        return -1;
    return 0;
}

int z80aw_close()
{
    close_serial_port();
    return 0;
}

bool z80aw_get_logging_to_stdout()
{
    return log_to_stdout;
}

bool z80aw_get_assert_empty_buffer()
{
    return assert_empty_buffer;
}

Z80AW_RegisterFetchMode z80aw_get_register_fetch_mode()
{
    return register_fetch_mode;
}

int z80aw_set_logging_to_stdout(bool v)
{
    log_to_stdout = v;
    return 0;
}

int z80aw_set_assert_empty_buffer(bool v)
{
    assert_empty_buffer = v;
    return 0;
}

int z80aw_set_register_fetch_mode(Z80AW_RegisterFetchMode mode)
{
    zsend_noreply(Z_REGFETCH_MODE | Z_COMMAND);
    zsend_noreply(mode);
    int r = zrecv_response();
    if (r == Z_EMULATOR_ONLY) {
        ERROR("The register emulator mode can only be set when using the emulator.");
    } else if (r != Z_OK) {
        ERROR("Unknown error when setting emulator mode.");
    }
    
    regfetch_enabled = (mode != Z80AW_REGFETCH_DISABLED);
    register_fetch_mode = mode;
    return 0;
}

int z80aw_set_error_callback(void (*error_cb_)(const char* description, void* data), void* data)
{
    error_cb = error_cb_;
    error_data = data;
    return 0;
}

static void continue_execution(int sig)  // called when signal SIGUSR1 is received from emulator
{
    (void) sig;
    wait_for_emulator = false;
    printf("Signal received from emulator.\n");
}

int z80aw_initialize_emulator(const char* emulator_path, char* serial_port_buf, size_t serial_port_buf_sz, const char* disk_image_path)
{
    char serial_port[256] = "";
    
    if (signal(SIGUSR1, continue_execution) == SIG_ERR) {
        ERROR("Could not setup signal handler.\n");
    }
    
    // initialize emulator
    char cmdbuf[1024];
    snprintf(cmdbuf, sizeof cmdbuf, "%s/emulator", emulator_path);
    if (access(cmdbuf, F_OK) != 0) {
        ERROR("Could not find emulator executable at path '%s'", cmdbuf);
    }
    
    pid_t parent_pid = getpid();
    pid_t emulator_pid = fork();
    int r = 0;
    if (emulator_pid == -1) {
        ERROR("Could not fork.");
    } else if (emulator_pid == 0) {
        // child code
        // prepare emulator to die with parent
        if (prctl(PR_SET_PDEATHSIG, SIGTERM) < 0)
            abort();
        if (getppid() != parent_pid)
            abort();
    
        // start emulator
        char pid_s[16];
        snprintf(pid_s, sizeof pid_s, "%d", parent_pid);
        printf("Starting emulator with '%s -p %s'\n", cmdbuf, pid_s);
        if (disk_image_path)
            r = execl(cmdbuf, cmdbuf, "-p", pid_s, "-d", disk_image_path, NULL);
        else
            r = execl(cmdbuf, cmdbuf, "-p", pid_s, NULL);
        if (r < 0) {
            ERROR("Could not initialize emulator: %s", strerror(errno));
        }
    } else {
        // parent code
        while (wait_for_emulator);   // this variable is swapped when signal SIGUSR1 is received from emulator
        wait_for_emulator = true;   // reset state
    
        // read port from file created by the emulator
        FILE* f = fopen("./.port", "r");
        if (!f) {
            ERROR("Could not open port file from emulator");
        }
        fread(serial_port, sizeof serial_port, 1, f);
        fclose(f);
        unlink("./.port");
    
        snprintf(serial_port_buf, serial_port_buf_sz, "%s", serial_port);
    }
    
    return 0;
}

void z80aw_set_error(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(last_error, sizeof last_error, fmt, ap);
    if (log_to_stdout)
        printf("\e[0;31mERROR: %s\e[0m\n", last_error);
    va_end(ap);
    
    if (error_cb)
        error_cb(last_error, error_data);
}

const char* z80aw_last_error()
{
    return last_error;
}

Z80AW_ControllerInfo z80aw_controller_info()
{
    Z80AW_ControllerInfo c;
    zsend_noreply(Z_CTRL_INFO | Z_COMMAND);
    c.free_memory = zrecv16_response();
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
    int times = 0;
    uint16_t checksum = z80aw_checksum(sz, data);
try_again:
    zsend_noreply(Z_WRITE_BLOCK | Z_COMMAND);
    zsend_noreply(addr & 0xff);
    zsend_noreply(addr >> 8);
    zsend_noreply(sz & 0xff);
    zsend_noreply(sz >> 8);
    for (uint16_t i = 0; i < sz; ++i)
        zsend_noreply(data[i]);
    uint8_t status = zrecv_response();
    uint16_t rchecksum = zrecv16();
    if (status == Z_INCORRECT_BUS) {
        ERROR("Bus is in an incorrect state when writing to memory.");
    }
    if (checksum != rchecksum) {
        if (times++ < 3) {
            fprintf(stderr, "Could not write to memory, trying %d/3...\n", times-1);
            goto try_again;
        }
        ERROR("When writing to memory, received checksum (0x%x) does not match with calculated checksum (0x%x).", rchecksum, checksum);
    }
    z_assert_empty_buffer();
    return 0;
}

int z80aw_read_block(uint16_t addr, uint16_t sz, uint8_t* data)
{
    zsend_noreply(Z_READ_BLOCK | Z_COMMAND);
    zsend_noreply(addr & 0xff);
    zsend_noreply(addr >> 8);
    zsend_noreply(sz & 0xff);
    zsend_noreply(sz >> 8);
    uint8_t status = zrecv_response();
    for (uint16_t i = 0; i < sz; ++i)
        data[i] = zrecv();
    if (status == Z_INCORRECT_BUS)
        ERROR("Bus is in an incorrect state when reading from memory.");
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

int z80aw_upload_compiled(DebugInformation const* di, void (*upload_callback)(void* data, float perc), void* data)
{
    z80aw_cpu_powerdown();

    if (upload_callback)
        upload_callback(data, 0.f);
    
    // write data
    size_t i = 0;
    for (Binary const* bin = debug_binary(di, i); bin; bin = debug_binary(di, ++i)) {
        int r = z80aw_write_block(bin->addr, bin->sz, bin->data);
        if (r != 0)
            return r;
        if (upload_callback)
            upload_callback(data, (float) i / (float) debug_binary_count(di));
    }
    z_assert_empty_buffer();
    
    // write checksum
    if (upload_callback)
        upload_callback(data, 0.99f);
    uint16_t chk = debug_binary_checksum(di);
    uint8_t bdata[] = { chk & 0xff, chk >> 8 };
    int r = z80aw_write_block(UPLOAD_CHECKSUM_LOCATION, 2, bdata);
    if (r != 0)
        return r;
    z_assert_empty_buffer();
    if (upload_callback)
        upload_callback(data, 1.f);
    
    // reset
    z80aw_remove_all_breakpoints();
    // z80aw_cpu_reset();
    z_assert_empty_buffer();
    
    return 0;
}

bool z80aw_is_uploaded(DebugInformation const* di)
{
    uint8_t data[2];
    z80aw_read_block(UPLOAD_CHECKSUM_LOCATION, 2, data);
    uint16_t chk = debug_binary_checksum(di);
    
    return (data[0] == (chk & 0xff)) && (data[1] == (chk >> 8));
}

int z80aw_simple_compilation(const char* code, char* err_buf, size_t err_buf_sz)
{
    struct DebugInformation* di = z80aw_simple_compilation_debug(code, err_buf, err_buf_sz);
    if (di) {
        debug_free(di);
        return 0;
    } else {
        return -1;
    }
}

DebugInformation* z80aw_simple_compilation_debug(const char* code, char* err_buf, size_t err_buf_sz)
{
    // create temporary directory
    char tmpdir[256];
    snprintf(tmpdir, sizeof tmpdir, "%s/z80_XXXXXX", P_tmpdir);
    if (mkdtemp(tmpdir) == NULL)
        ERROR_N("Could not create temp dir.");
    
    // create source file
    char filename[512];
    snprintf(filename, sizeof filename, "%s/project.z80", tmpdir);
    FILE* f = fopen(filename, "w");
    if (!f)
        ERROR_N("Could not create source file.");
    fprintf(f, "%s", code);
    fclose(f);
    
    // create project file
    char pfilename[512];
    snprintf(pfilename, sizeof pfilename, "%s/project.toml", tmpdir);
    f = fopen(pfilename, "w");
    fprintf(f, "sources = [{ source = \"project.z80\", address = 0x0 }]");
    fclose(f);
    
    // compile
    DebugInformation* di = compile_vasm(pfilename);
    if (!di) {
        snprintf(err_buf, err_buf_sz, "%s", last_error);
        return NULL;
    }
    bool success = debug_output(di, err_buf, err_buf_sz);
    if (success) {
        z80aw_upload_compiled(di, NULL, NULL);
        z_assert_empty_buffer();
    } else {
        debug_free(di);
        di = NULL;
    }
    
    // cleanup
    unlink(filename);
    unlink(pfilename);
    rmdir(tmpdir);
    
    return di;
}

int z80aw_cpu_reset()
{
    int r = zsend_expect(Z_RESET | Z_COMMAND, Z_OK);
    if (r == 0)
        z80_power = true;
    z_assert_empty_buffer();
    return r;
}

int z80aw_cpu_powerdown()
{
    int r = zsend_expect(Z_POWERDOWN | Z_COMMAND, Z_OK);
    if (r == 0)
        z80_power = false;
    z_assert_empty_buffer();
    return r;
}

int z80aw_cpu_pc()
{
    int r = zsend_noreply(Z_PC | Z_COMMAND);
    uint16_t pc = zrecv16();
    return r < 0 ? 0 : pc;
}

static void recv_registers(Z80AW_Registers* reg)
{
    uint8_t r[27];
    for (size_t i = 0; i < 27; ++i)
        r[i] = zrecv();
    if (reg) {
        *reg = (Z80AW_Registers) {
                .AF = (uint16_t) (r[1] | r[0] << 8),
                .BC = (uint16_t) (r[3] | r[2] << 8),
                .DE = (uint16_t) (r[5] | r[4] << 8),
                .HL = (uint16_t) (r[7] | r[6] << 8),
                .AFx = (uint16_t) (r[9] | r[8] << 8),
                .BCx = (uint16_t) (r[11] | r[10] << 8),
                .DEx = (uint16_t) (r[13] | r[12] << 8),
                .HLx = (uint16_t) (r[15] | r[14] << 8),
                .IX = (uint16_t) (r[16] | r[17] << 8),
                .IY = (uint16_t) (r[18] | r[19] << 8),
                .PC = (uint16_t) (r[20] | r[21] << 8),
                .SP = (uint16_t) (r[22] | r[23] << 8),
                .R = r[24],
                .I = r[25],
                .HALT = r[26],
                .valid = true,
        };
    }
}

int z80aw_cpu_registers(Z80AW_Registers* reg)
{
    zsend_noreply(Z_REGISTERS | Z_COMMAND);
    int r = zrecv_response();
    if (r == Z_OK) {
        recv_registers(reg);
    } else if (r == Z_EMULATOR_ONLY) {
        ERROR("Registers can only be fetched from the emulator.");
    } else {
        ERROR("Invalid response from register fetch.");
    }
    return 0;
}

int z80aw_cpu_nmi()
{
    int r = zsend_expect(Z_NMI | Z_COMMAND, Z_OK);
    z_assert_empty_buffer();
    return r;
}

static int z80aw_cpu_step_debug(Z80AW_Registers* reg, uint8_t* printed_char)
{
    if (z80_power == false) {
        ERROR("CPU is not powered up.");
    }
    
    int resp = zsend_noreply(Z_STEP | Z_COMMAND);
    if (resp != 0)
        return resp;
    
    recv_registers(reg);
    
    uint8_t pchar = zrecv();
    if (printed_char)
        *printed_char = pchar;
    
    z_assert_empty_buffer();
    return 0;
}

static int z80aw_cpu_step_disabled(uint8_t* printed_char)
{
    if (z80_power == false) {
        ERROR("CPU is not powered up.");
    }
    
    int r = zsend_noreply(Z_STEP | Z_COMMAND);
    if (r != 0)
        return r;
    
    uint8_t c = zrecv();
    if (printed_char)
        *printed_char = c;
    
    z_assert_empty_buffer();
    return 0;
}

int z80aw_cpu_step(Z80AW_Registers* reg, uint8_t* printed_char)
{
    if (regfetch_enabled) {
        return z80aw_cpu_step_debug(reg, printed_char);
    } else {
        if (reg)
            reg->valid = false;
        return z80aw_cpu_step_disabled(printed_char);
    }
}

int z80aw_keypress(uint8_t key)
{
    zsend_noreply(Z_KEYPRESS | Z_COMMAND);
    int r = zsend_expect(key, Z_OK);
    z_assert_empty_buffer();
    return r;
}

int z80aw_add_breakpoint(uint16_t addr)
{
    zsend_noreply(Z_ADD_BKP | Z_COMMAND);
    zsend_noreply(addr & 0xff);
    zsend_noreply(addr >> 8);
    int r = zrecv_response();
    if (r == Z_TOO_MANY_BKPS) {
        ERROR("Too many breakpoints.");
    }
    z_assert_empty_buffer();
    return 0;
}

int z80aw_remove_breakpoint(uint16_t addr)
{
    zsend_noreply(Z_REMOVE_BKP | Z_COMMAND);
    zsend_noreply(addr & 0xff);
    int r = zsend_expect(addr >> 8, Z_OK);
    z_assert_empty_buffer();
    return r;
}

int z80aw_remove_all_breakpoints()
{
    int r = zsend_expect(Z_REMOVE_ALL_BKPS | Z_COMMAND, Z_OK);
    z_assert_empty_buffer();
    return r;
}

int z80aw_query_breakpoints(uint16_t* addr, size_t addr_sz)
{
    zsend_noreply(Z_QUERY_BKPS | Z_COMMAND);
    int count = zrecv();
    if (count < 0)
        return -1;
    for (ssize_t i = 0; i < count; ++i) {
        uint16_t a = zrecv16();
        if (i < (ssize_t) addr_sz && addr)
            addr[i] = a;
    }
    z_assert_empty_buffer();
    return count;
}

int z80aw_cpu_next()
{
    if (z80_power == false) {
        ERROR("CPU is not powered up.");
    }
    
    int r = zsend_expect(Z_NEXT | Z_COMMAND, Z_OK);
    z_assert_empty_buffer();
    return r;
}

int z80aw_cpu_continue()
{
    if (z80_power == false) {
        ERROR("CPU is not powered up.");
    }
    
    int r = zsend_expect(Z_CONTINUE | Z_COMMAND, Z_OK);
    z_assert_empty_buffer();
    return r;
}

int z80aw_cpu_stop()
{
    int r = zsend_expect(Z_STOP | Z_COMMAND, Z_OK);
    z_assert_empty_buffer();
    return r;
}

Z80AW_Event z80aw_last_event()
{
    zsend_expect(Z_LAST_EVENT | Z_COMMAND, Z_OK);
    
    Z80AW_Event event;
    event.char_printed = zrecv();
    event.bkp_reached = zrecv();
    event.pc = zrecv16();
    z_assert_empty_buffer();
    return event;
}

int z80aw_finalize_emulator()
{
    int r = zsend_expect(Z_EXIT_EMULATOR | Z_COMMAND, Z_OK);
    z_assert_empty_buffer();
    return r;
}

bool z80aw_has_disk()
{
    zsend_noreply(Z_HAS_DISK | Z_COMMAND);
    int r = zrecv_response();
    z_assert_empty_buffer();
    return !(r == Z_OK);
}

int
z80aw_write_disk_block(uint32_t block, uint8_t const* data)
{
    zsend_noreply(Z_WRITE_DISK | Z_COMMAND);
    zsend_noreply(block & 0xff);
    zsend_noreply(block >> 8);
    zsend_noreply(block >> 16);
    for (int i = 0; i < 512; ++i)
        zsend_noreply(data[i]);
    int c = zrecv_response();
    z_assert_empty_buffer();
    return !(c == Z_OK);
}

int
z80aw_read_disk_block(uint32_t block, uint8_t* data)
{
    zsend_noreply(Z_READ_DISK | Z_COMMAND);
    zsend_noreply(block & 0xff);
    zsend_noreply(block >> 8);
    if (zsend_expect(block >> 16, Z_OK) != 0)
        return -1;
    for (int i = 0; i < 512; ++i)
        data[i] = zrecv();
    return 0;
}
