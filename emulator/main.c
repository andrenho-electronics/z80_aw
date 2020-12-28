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

static uint8_t memory[64 * 1024];
static Z80 z80;

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
    /*
    if ((Port & 0xff) == 0x0)
        global_terminal->print_char(Value);
    */
}

byte InZ80(word Port)
{
    // return global_terminal->last_keypress();
    return 0;
}

word LoopZ80(Z80 *R)
{
    /*
    if (global_terminal->keyboard_interrupt()) {
        global_terminal->clear_keyboard_interrupt();
        return 0xcf;
    }
     */
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

uint8_t recv(bool* eof) {
    uint8_t c;
    int r = read(master, &c, 1);
    if (r > 0) {
        if (last_comm != 'R')
            printf("\n\x1b[32m< ");
        printf("[%02X] ", c);
        fflush(stdout);
        last_comm = 'R';
        return c;
    } else {
        if (eof)
            *eof = true;
        return 0;
    }
}

void send(uint8_t c) {
    if (last_comm != 'W')
        printf("\n\x1b[31m> ");
    printf("[%02X] ", c);
    fflush(stdout);
    last_comm = 'W';
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
            for (uint16_t i = 0; i < sz; ++i)
                send(memory[addr + i]);
        }
            break;
        case C_RESET:
            ResetZ80(&z80);
            send(C_OK);
            break;
        case C_STEP:
            RunZ80(&z80);
            send(z80.PC.W & 0xff);
            send(z80.PC.W << 8);
            send(0);  // TODO - send printed character
            break;
        default:
            fprintf(stderr, "Unexpected byte.");
            *exit = true;
    }
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
