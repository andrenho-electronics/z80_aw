#define _GNU_SOURCE 1

#include <pty.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "protocol.h"
#include "z80/Z80.h"

//
// Z80 EMULATOR CALLBACKS
//

#define MAX_BREAKPOINTS 10

typedef enum { M_STEP, M_CONTINUE } Mode;

static uint8_t  memory[64 * 1024];
static Z80      z80;
static bool     keyboard_interrupt = false;
static uint8_t  last_keypress = 0;
static int      last_printed_char = -1;
static uint16_t breakpoints[MAX_BREAKPOINTS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static Mode     mode = M_STEP;

static void send(uint8_t c);
static uint8_t recv_nonblock();
static void keypress();

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
    if ((Port & 0xff) == 0x0) {
        last_printed_char = Value;
        if (mode == M_CONTINUE) {
            send(C_PRINT);
            send(last_printed_char);
        }
    }
}

byte InZ80(word Port)
{
    if ((Port & 0xff) == 0x1) {
        return (uint8_t) last_keypress;
    }
    return 0;
}

bool is_breakpoint(word w);

word done();

word LoopZ80(Z80 *R)
{
    if (keyboard_interrupt) {
        keyboard_interrupt = false;
        return 0xcf;
    }
    if (mode == M_STEP) {
        return INT_QUIT;
    } else if (mode == M_CONTINUE) {
        uint8_t c = recv_nonblock();
        switch (c) {
            case 0:
                break;
            case C_KEYPRESS:
                keypress();
                break;
            case C_BREAK:
                return done();
            default:
                send(C_ERR);
                exit(EXIT_FAILURE);
        }
        if (is_breakpoint(R->PC.W)) {
            return done();
        } else {
            return INT_NONE;
        }
    }
}

word done()
{
    send(C_DONE);
    send(z80.PC.W & 0xff);
    send(z80.PC.W >> 8);
    return INT_QUIT;
}

void PatchZ80(Z80 *R)
{
}

//
// SERIAL COMMUNICATION
//

static int master;
static char last_comm = '\0';

static void z80_continue();

static void keypress();

uint8_t recv(bool* eof) {
    uint8_t c;
    int r = read(master, &c, 1);
    if (r > 0) {
        /*
        if (last_comm != 'R')
            printf("\n\x1b[32m< ");
        printf("[%02X] ", c);
        fflush(stdout);
        last_comm = 'R';
        */
        return c;
    } else {
        if (eof)
            *eof = true;
        return 0;
    }
}

static uint8_t recv_nonblock() {
    uint8_t c;
    int r = read(master, &c, 1);
    if (r == 0)
        return 0;
    else
        return c;
}

uint16_t recv16() {
    uint16_t a = recv(NULL);
    uint16_t b = recv(NULL);
    return a | (b << 8);
}

static void send(uint8_t c) {
    /*
    if (last_comm != 'W')
        printf("\n\x1b[31m> ");
    printf("[%02X] ", c);
    fflush(stdout);
    last_comm = 'W';
     */
    write(master, &c, 1);
}

//
// COMMANDS
//

static bool parse_input(bool* exit)
{
    uint8_t c = recv(exit);
    switch (c) {
         case C_ACK:
             send(C_OK);
             break;
         case C_RAM_BYTE: {
             uint8_t a = recv(NULL);
             uint8_t b = recv(NULL);
             printf("Read RAM byte (addr: 0x%04x)\n", a | (b << 8));
             send(memory[a | (b << 8)]);
         }
             break;
         case C_RAM_BLOCK: {
             uint8_t a = recv(NULL);
             uint8_t b = recv(NULL);
             uint8_t c1 = recv(NULL);
             uint8_t c2 = recv(NULL);
             uint16_t addr = a | (b << 8);
             uint16_t sz = c1 | (c2 << 8);
             printf("Read RAM block (addr: 0x%04x, size: 0x%x)\n", addr, sz);
             for (uint16_t i = 0; i < sz; ++i)
                 send(memory[addr + i]);
         }
             break;
         case C_RESET:
             printf("Reset\n");
             ResetZ80(&z80);
             send(C_OK);
             break;
         case C_STEP:
             mode = M_STEP;
             RunZ80(&z80);
             send(z80.PC.W & 0xff);
             send(z80.PC.W >> 8);
             if (last_printed_char != -1) {
                 printf("Print char 0x%02x (%c)\n", last_printed_char, last_printed_char);
                 send(last_printed_char);
                 last_printed_char = -1;
             } else {
                 send(0);
             }
             break;
        case C_CONTINUE:
            mode = M_CONTINUE;
            RunZ80(&z80);
            break;
        case C_REGISTERS:
            printf("Update registers\n");
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
            break;
        case C_UPLOAD:
            printf("Upload: block sizes: ");
            send(C_UPLOAD_ACK);
            for (;;) {
                uint16_t addr = recv16();
                send(C_UPLOAD_ACK);
                uint16_t sz = recv16();
                printf("%d ", sz);
                if (sz == 0)
                    break;
                uint16_t checksum1 = 0, checksum2 = 0;
                for (uint16_t i = 0; i < sz; ++i) {
                    memory[addr + i] = recv(NULL);
                    checksum1 = (checksum1 + memory[addr + i]) % 255;
                    checksum2 = (checksum2 + checksum1) % 255;
                }
                send(checksum1);
                send(checksum2);
                usleep(100000);   // simulate slow ROM
            }
            send(C_UPLOAD_ACK);
            printf("\n");
            ResetZ80(&z80);
            break;
        case C_KEYPRESS:
            keypress();
            break;
        case C_ADD_BKP: {
                bool found = false;
                for (int i = 0; i < MAX_BREAKPOINTS; ++i) {
                    if (breakpoints[i] == 0) {
                        breakpoints[i] = recv16();
                        send(C_OK);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    recv16();
                    send(C_ERR);
                }
            }
            break;
        case C_REMOVE_BKP: {
                uint16_t addr = recv16();
                for (int i = 0; i < MAX_BREAKPOINTS; ++i) {
                    if (breakpoints[i] == addr)
                        breakpoints[i] = 0;
                }
            }
            break;
        default:
            fprintf(stderr, "Unexpected byte.");
            *exit = true;
    }
}

static void keypress()
{
    last_keypress = recv(NULL);
    printf("Keypress: 0x%02x\n", last_keypress);
    keyboard_interrupt = true;
    send(C_OK);
}

bool is_breakpoint(word w)
{
    for (size_t i = 0; i < MAX_BREAKPOINTS; ++i)
        if (breakpoints[i] != 0 && breakpoints[i] != w)
            return true;
    return false;
}

//
// MAIN PROCEDURE
//

int main()
{
    int slave;
    char name[256];
    
    int e = openpty(&master, &slave, name, NULL, NULL);
    if (e < 0) {
        perror("openpty");
        return EXIT_FAILURE;
    }
    
    printf("Open connection to: %s\n", name);
    
    z80.TrapBadOps = 1;
    ResetZ80(&z80);
    
    bool eof = false;
    while (!eof)
        parse_input(&eof);
    
    close(slave);
    close(master);
}
