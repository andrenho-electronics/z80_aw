#include "z80aw.h"

#include <stdarg.h>
#include <stdio.h>
#include <protocol.h>
#include <signal.h>
#include <unistd.h>

#include "comm.h"
#include "z80aw_priv.h"

static char last_error[256] = "No error.";
static bool log_to_stdout = false;
static int  timeout = 5;
static bool wait_for_emulator = true;

void z80aw_init(Z80AW_Config* cfg)
{
    open_serial_port(cfg->serial_port, cfg->log_to_stdout, cfg->serial_timeout);
    timeout = cfg->serial_timeout < 5 ? 5 : cfg->serial_timeout;
    log_to_stdout = cfg->log_to_stdout;
}

void z80aw_close()
{
    close_serial_port();
}

static void continue_execution(int sig)  // called when signal SIGUSR1 is received from emulator
{
    (void) sig;
    wait_for_emulator = false;
    printf("Signal received from emulator.\n");
}

int z80aw_initialize_emulator(const char* emulator_path, char* serial_port_buf, size_t serial_port_buf_sz)
{
    static char serial_port[256];
    
    if (signal(SIGUSR1, continue_execution) == SIG_ERR)
        ERROR("Could not setup signal handler.\n");
    
    // initialize emulator
    pid_t my_pid = getpid();
    pid_t emulator_pid = fork();
    if (emulator_pid == 1) {
        ERROR("Could not fork.");
    } else if (emulator_pid == 0) {
        char pid_s[16], cmdbuf[1024];
        snprintf(pid_s, sizeof pid_s, "%d", my_pid);
        snprintf(cmdbuf, sizeof cmdbuf, "%s/emulator", emulator_path);
        printf("Starting emulator with '%s -p %s'\n", cmdbuf, pid_s);
        execl(cmdbuf, cmdbuf, "-p", pid_s, NULL);
    }
    
    while (wait_for_emulator);   // this variable is swapped when signal SIGUSR1 is received from emulator
    
    // read port from file created by the emulator
    FILE* f = fopen("./.port", "r");
    if (!f)
        ERROR("Could not open port file from emulator");
    fread(serial_port, sizeof serial_port, sizeof serial_port - 1, f);
    fclose(f);
    unlink("./.port");
    
    snprintf(serial_port_buf, serial_port_buf_sz, "%s", serial_port);
    
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
    
    // reset
    z80aw_cpu_reset();
    
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
    // create temporary directory
    char tmpdir[256];
    snprintf(tmpdir, sizeof tmpdir, "%s/z80_XXXXXX", P_tmpdir);
    if (mkdtemp(tmpdir) == NULL)
        ERROR("Could not create temp dir.");
    
    // create source file
    char filename[512];
    snprintf(filename, sizeof filename, "%s/project.z80", tmpdir);
    FILE* f = fopen(filename, "w");
    if (!f)
        ERROR("Could not create source file.");
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
        return -1;
    }
    bool success = debug_output(di, err_buf, err_buf_sz);
    z80aw_upload_compiled(di);
    debug_free(di);
    
    // cleanup
    unlink(filename);
    unlink(pfilename);
    rmdir(tmpdir);
    
    return success ? 0 : -1;
}

int z80aw_cpu_reset()
{
    return zsend_expect(Z_RESET, Z_OK);
}

int z80aw_cpu_registers(Z80AW_Registers* reg)
{
    int resp = zsend_noreply(Z_REGISTERS);
    if (resp != 0)
        return resp;
    
    uint8_t r[27];
    for (size_t i = 0; i < 27; ++i)
        r[i] = zrecv();
    
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
    };
    
    return 0;
}

int z80aw_cpu_step(uint8_t* printed_char)
{
    int r = zsend_noreply(Z_STEP);
    if (r != 0)
        return r;
    
    uint8_t c = zrecv();
    if (printed_char)
        *printed_char = c;
    
    return 0;
}