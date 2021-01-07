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

//
// command line options
//

void get_options(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                test_pid = strtol(optarg, NULL, 10);
                printf("emulator: Being run from test process on pid %d.\n", test_pid);
                break;
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
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(master, &set);
    timeout.tv_sec = 1;    // 5 seconds
    timeout.tv_usec = 0;
    while (select(FD_SETSIZE, &set, NULL, NULL, &timeout) == 0)
        exit_if_parent_died();
    
    uint8_t c;
    read(master, &c, 1);
    return c;
}

static uint8_t recv_nowait() {
    uint8_t c;
    if (read(master, &c, 1) == 1)
        return c;
    else
        return 0;
}

static uint16_t recv16() {
    int a = recv();
    int b = recv();
    return a | (b << 8);
}

static void send(uint8_t c) {
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

static void send_registers()
{
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
    switch (c) {
        case 0:
            break;
        case Z_ACK_REQUEST:
            send(Z_ACK_RESPONSE);
            break;
        case Z_EXIT_EMULATOR:
            send(Z_OK);
            usleep(200000);
            exit(EXIT_SUCCESS);
        case Z_CTRL_INFO:
            send16(0x800);
            break;
        case Z_READ_BLOCK: {
                uint16_t addr = recv16();
                uint16_t sz = recv16();
                for (size_t i = 0; i < sz; ++i)
                    send(memory[i + addr]);
            }
            break;
        case Z_WRITE_BLOCK: {
                uint16_t addr = recv16();
                uint16_t sz = recv16();
                for (size_t i = 0; i < sz; ++i)
                    memory[i + addr] = recv();
                uint16_t chk = checksum(sz, &memory[addr]);
                send16(chk);
            }
            break;
        case Z_RESET:
            ResetZ80(&z80);
            send(Z_OK);
            break;
        case Z_REGISTERS:
            send_registers();
            break;
        case Z_STEP:
            RunZ80(&z80);
            send(last_printed_char);
            last_printed_char = 0;
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
        case Z_CONTINUE:
            send(Z_OK);
            continue_mode = true;
            RunZ80(&z80);
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
        case Z_PIN_STATUS:
            send_pins();
            break;
        case Z_CYCLE:
            ++cycle_number;
            cpu_random_pins = ((uint64_t) rand() << 48) | ((uint64_t) rand() << 32) | ((uint64_t) rand() << 16) | (uint16_t) rand();
            send(Z_OK);
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
    
    ResetZ80(&z80);
    
    while (1)
        command_loop();
}