#include <pty.h>

#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "protocol.h"
#include "z80/Z80.h"

static int     master;
static char    serial_port_name[256];
static int     test_pid = 0;

#define MAX_BREAKPOINTS 16

//
// globals
//

Z80      z80;
uint8_t  memory[64 * 1024];
uint8_t  last_printed_char = 0;
uint8_t  last_keypress = 0;
bool     keyboard_interrupt = false;
uint16_t breakpoints[MAX_BREAKPOINTS] = { 0 };
bool     continue_mode = false;
uint8_t  last_event = Z_OK;
uint32_t cycle_number = 0;
uint64_t cpu_random_pins = 0;
bool     log_to_stdout = false;
bool     load_registers_native = true;

typedef enum { NOT_WAITING, SEND_NMI, WAITING_IO, WAITING_RETI } RegisterLoadState;
RegisterLoadState register_load_state = NOT_WAITING;
uint16_t          register_stack_location = 0;

//
// command line options
//

void get_options(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "hlzp:")) != -1) {
        switch (opt) {
            case 'p':
                test_pid = strtol(optarg, NULL, 10);
                printf("emulator: Being run from test process on pid %d.\n", test_pid);
                break;
            case 'l':
                log_to_stdout = true;
                break;
            case 'z':
                load_registers_native = false;
                break;
            case 'h':
                printf("Usage: %s [-p PID] [-l]\n", argv[0]);
                printf("     -p      Parent pid. Use this when started by another process.\n");
                printf("     -z      When used, will fire a NMI on Z80 to load registers. Otherwise, use emulator native interface.\n");
                printf("     -l      Log bytes to stdout\n");
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "emulator: Invalid option.\n");
                exit(EXIT_FAILURE);
        }
    }
}

//
// serial communication
//

void open_serial()
{
    int slave;
    
    int e = openpty(&master, &slave, serial_port_name, NULL, NULL);
    if (e < 0) {
        perror("openpty");
        exit(EXIT_FAILURE);
    }
    if (fcntl(master, F_SETFL, FNDELAY) < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    
    printf("emulator: Listening in port: %s\n", serial_port_name);
}

static void exit_if_parent_died()
{
    if (getppid() == 1)   // parent has died, lets die too
        exit(1);
}

static uint8_t recv() {
    /*
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(master, &set);
    timeout.tv_sec = 1;    // 5 seconds
    timeout.tv_usec = 0;
    while (select(FD_SETSIZE, &set, NULL, NULL, &timeout) == 0)
        exit_if_parent_died();
    */
    
    uint8_t c;
    int r = 0;
    while (r != 1)
        r = read(master, &c, 1);
    if (log_to_stdout) {
        printf("\e[0;33m%02X \e[0m", c);
        fflush(stdout);
    }
    return c;
}

static uint8_t recv_nowait() {
    uint8_t c;
    if (read(master, &c, 1) == 1) {
        if (log_to_stdout) {
            printf("\e[0;33m*%02X \e[0m", c);
            fflush(stdout);
        }
        return c;
    } else {
        return 0;
    }
}

static uint16_t recv16() {
    int a = recv();
    int b = recv();
    return a | (b << 8);
}

static void send(uint8_t c) {
    if (log_to_stdout) {
        printf("\e[0;34m%02X \e[0m", c);
        fflush(stdout);
    }
    write(master, &c, 1);
}

static void send16(uint16_t v) {
    send(v & 0xff);
    send(v >> 8);
}

static void send_port_to_test()
{
    FILE* fp = fopen("./.port", "w");
    fprintf(fp, "%s", serial_port_name);
    fclose(fp);
    printf("emulator: Sending signal to process %d...\n", test_pid);
    kill(test_pid, SIGUSR1);
}

//
// utils
//

static uint16_t checksum(size_t sz, uint8_t const* data)
{
    uint16_t checksum1 = 0, checksum2 = 0;
    for (size_t i = 0; i < sz; ++i) {
        checksum1 = (checksum1 + data[i]) % 255;
        checksum2 = (checksum2 + checksum1) % 255;
    }
    return checksum1 | (checksum2 << 8);
}

static void send_pins()
{
    send(cycle_number);
    send(cycle_number >> 8);
    send(cycle_number >> 16);
    send(cycle_number >> 24);
    
    // address
    send(cpu_random_pins);
    send(cpu_random_pins >> 8);
    
    // data
    send(cpu_random_pins >> 16);
    
    // pins
    send(cpu_random_pins >> 24);
    send(cpu_random_pins >> 32);
}

//
// load registers
//

static void step_debug_native()
{
    RunZ80(&z80);
    send(z80.AF.B.h);
    send(z80.AF.B.l);
    send(z80.BC.B.h);
    send(z80.BC.B.l);
    send(z80.DE.B.h);
    send(z80.DE.B.l);
    send(z80.HL.B.h);
    send(z80.HL.B.l);
    send(z80.AF1.B.h);
    send(z80.AF1.B.l);
    send(z80.BC1.B.h);
    send(z80.BC1.B.l);
    send(z80.DE1.B.h);
    send(z80.DE1.B.l);
    send(z80.HL1.B.h);
    send(z80.HL1.B.l);
    send(z80.IX.W & 0xff);
    send(z80.IX.W >> 8);
    send(z80.IY.W & 0xff);
    send(z80.IY.W >> 8);
    send(z80.PC.W & 0xff);
    send(z80.PC.W >> 8);
    send(z80.SP.W & 0xff);
    send(z80.SP.W >> 8);
    send(z80.R);
    send(z80.I);
    send((z80.IFF & IFF_HALT) ? 1 : 0);
    send(last_printed_char);
    last_printed_char = 0;
}

static void step_debug_nmi()
{
    register_load_state = SEND_NMI;
    register_stack_location = 0;
    bool halt = (z80.IFF & IFF_HALT);
    
    // Z80 will run NMI interrupt and, at start, add registers to stack
    while (register_load_state == SEND_NMI)
        RunZ80(&z80);
   
    // Z80 will tell the location of the stack
    while (register_load_state == WAITING_IO)
        RunZ80(&z80);
    
    // load registers
    uint16_t ir = memory[z80.SP.W] | (memory[z80.SP.W + 1] << 8);
    uint16_t iy = memory[z80.SP.W + 2] | (memory[z80.SP.W + 3] << 8);
    uint16_t ix = memory[z80.SP.W + 4] | (memory[z80.SP.W + 5] << 8);
    uint16_t hlx = memory[z80.SP.W + 6] | (memory[z80.SP.W + 7] << 8);
    uint16_t dex = memory[z80.SP.W + 8] | (memory[z80.SP.W + 9] << 8);
    uint16_t bcx = memory[z80.SP.W + 10] | (memory[z80.SP.W + 11] << 8);
    uint16_t afx = memory[z80.SP.W + 12] | (memory[z80.SP.W + 13] << 8);
    uint16_t hl = memory[z80.SP.W + 14] | (memory[z80.SP.W + 15] << 8);
    uint16_t de = memory[z80.SP.W + 16] | (memory[z80.SP.W + 17] << 8);
    uint16_t bc = memory[z80.SP.W + 18] | (memory[z80.SP.W + 19] << 8);
    uint16_t af = memory[z80.SP.W + 20] | (memory[z80.SP.W + 21] << 8);
    
    // Z80 will restore everything to correct location and return from the interrupt
    uint16_t next_instruction;
    do {
        RunZ80(&z80);
        next_instruction = (memory[z80.PC.W] << 8) | memory[z80.PC.W + 1];
    } while (next_instruction != 0xed4d);  /* ED4D = RETI */
    RunZ80(&z80);  // execute RETI, return to original instruction
    register_load_state = NOT_WAITING;
    
    // send registers
    send(af >> 8);
    send(af & 0xff);
    send(bc >> 8);
    send(bc & 0xff);
    send(de >> 8);
    send(de & 0xff);
    send(hl >> 8);
    send(hl & 0xff);
    send(afx >> 8);
    send(afx & 0xff);
    send(bcx >> 8);
    send(bcx & 0xff);
    send(dex >> 8);
    send(dex & 0xff);
    send(hlx >> 8);
    send(hlx & 0xff);
    send(ix & 0xff);
    send(ix >> 8);
    send(iy & 0xff);
    send(iy >> 8);
    send(z80.PC.W & 0xff);
    send(z80.PC.W >> 8);
    send((register_stack_location + 0x18) & 0xff);
    send((register_stack_location + 0x18) >> 8);
    send(ir & 0xff);
    send(ir >> 8);
    send(halt);
    send(last_printed_char);
    last_printed_char = 0;
}

//
// breakpoints
//

bool bkp_add(uint16_t bkp)
{
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i) {
        if (breakpoints[i] == bkp)  // avoid duplicates
            return true;
        if (breakpoints[i] == 0) {
            breakpoints[i] = bkp;
            return true;
        }
    }
    return false;
}

void bkp_remove(uint16_t bkp)
{
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i)
        if (breakpoints[i] == bkp)
            breakpoints[i] = 0;
}

int bkp_query(uint16_t bkps[16])
{
    size_t j = 0;
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i) {
        if (breakpoints[i] != 0)
            bkps[j++] = breakpoints[i];
    }
    return j;
}

//
// Z80
//

void WrZ80(word Addr,byte Value)
{
    memory[Addr] = Value;
}

byte RdZ80(word Addr)
{
    return memory[Addr];
}

void OutZ80(word Port,byte Value)
{
    if ((Port & 0xff) == 0x0) {  // video
        last_printed_char = Value;
        if (continue_mode) {
            last_event = Z_PRINT_CHAR;
            continue_mode = false;
        }
    } else if ((Port & 0xff) == 0xfe) {  // load registers from stack - lower nibble
        register_stack_location = (register_stack_location & 0xff00) | Value;
    } else if ((Port & 0xff) == 0xff) {  // load registers from stack - higher nibble
        register_stack_location = (Value << 8) | (register_stack_location & 0xff);
        register_load_state = WAITING_RETI;
    }
}

byte InZ80(word Port)
{
    if ((Port & 0xff) == 0x1)  // keyboard
        return last_keypress;
    return 0;
}

word LoopZ80(Z80 *R)
{
    void command_loop();
    
    if (register_load_state == SEND_NMI) {
        register_load_state = WAITING_IO;
        return INT_NMI;
    }
    
    if (keyboard_interrupt) {
        keyboard_interrupt = false;
        return 0xcf;  // rst 0x8, returned by the keyboard controller
    }
    if (continue_mode) {
        command_loop();
        for (size_t i = 0; i < MAX_BREAKPOINTS; ++i)
            if (breakpoints[i] != 0 && breakpoints[i] == R->PC.W) {
                last_event = Z_BKP_REACHED;
                continue_mode = false;
                return INT_QUIT;
            }
        return INT_NONE;
    } else {
        return INT_QUIT;
    }
}

void PatchZ80(Z80 *R)
{
    (void) R;
}


//
// main loop
//

void command_loop()
{
    uint8_t c = continue_mode ? recv_nowait() : recv();
    if (c == 0)
        return;
    
    //
    // commands acceptable only in step mode
    //
    if (!continue_mode) {
        switch (c) {
            case Z_READ_BLOCK: {
                    uint16_t addr = recv16();
                    uint16_t sz = recv16();
                    for (size_t i = 0; i < sz; ++i)
                        send(memory[i + addr]);
                }
                return;
            case Z_WRITE_BLOCK: {
                    uint16_t addr = recv16();
                    uint16_t sz = recv16();
                    for (size_t i = 0; i < sz; ++i)
                        memory[i + addr] = recv();
                    uint16_t chk = checksum(sz, &memory[addr]);
                    send16(chk);
                }
                return;
            case Z_STEP_DEBUG:
                if (load_registers_native) {
                    step_debug_native();
                } else {
                    step_debug_nmi();
                }
                return;
            case Z_STEP:
                RunZ80(&z80);
                send(last_printed_char);
                last_printed_char = 0;
                return;
            case Z_CONTINUE:
                send(Z_OK);
                continue_mode = true;
                RunZ80(&z80);
                return;
            case Z_PIN_STATUS:
                send_pins();
                return;
            case Z_CYCLE:
                ++cycle_number;
                cpu_random_pins = ((uint64_t) rand() << 48) | ((uint64_t) rand() << 32) | ((uint64_t) rand() << 16) | (uint16_t) rand();
                send(Z_OK);
                return;
        }
    }
    
    //
    // commands acceptable both in continue and step mode
    //
    switch (c) {
        case Z_ACK_REQUEST:
            send(Z_ACK_RESPONSE);
            break;
        case Z_CTRL_INFO:
            send16(0x800);
            break;
        case Z_EXIT_EMULATOR:
            send(Z_OK);
            usleep(200000);
            exit(EXIT_SUCCESS);
        case Z_RESET:
            ResetZ80(&z80);
            send(Z_OK);
            break;
        case Z_KEYPRESS:
            last_keypress = recv();
            keyboard_interrupt = true;
            send(Z_OK);
            break;
        case Z_ADD_BKP:
            if (bkp_add(recv16()))
                send(Z_OK);
            else
                send(Z_TOO_MANY_BKPS);
            break;
        case Z_REMOVE_BKP:
            bkp_remove(recv16());
            send(Z_OK);
            break;
        case Z_REMOVE_ALL_BKPS:
            memset(breakpoints, 0, MAX_BREAKPOINTS * sizeof(uint16_t));
            send(Z_OK);
            break;
        case Z_QUERY_BKPS: {
                uint16_t bkps[MAX_BREAKPOINTS];
                int count = bkp_query(bkps);
                send(count);
                for (int i = 0; i < count; ++i)
                    send16(bkps[i]);
            }
            break;
        case Z_LAST_EVENT:
            send(last_event);
            if (last_event == Z_PRINT_CHAR)
                send(last_printed_char);
            last_event = Z_OK;
            break;
        case Z_STOP:
            continue_mode = false;
            send(Z_OK);
            break;
        case Z_PC:
            send16(z80.PC.W);
            break;
        default:
            fprintf(stderr, "emulator: Invalid command 0x%02X\n", c);
            send(Z_INVALID_CMD);
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    get_options(argc, argv);
    open_serial();
    if (test_pid)
        send_port_to_test();
    
    z80.Trace = 1;
    ResetZ80(&z80);
    
    while (1)
        command_loop();
}