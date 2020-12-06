#ifndef COMMLIB_H_
#define COMMLIB_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "messages.h"

typedef struct CommLib CommLib;

typedef struct {
    uint16_t cycle;
    uint16_t addr;
    uint8_t  data;
    Inputs   inputs;
} CL_Status;

// initialization
CommLib* cl_init(const char* comfile, int speed);
void     cl_free(CommLib* cl);

// debug
void     cl_set_debug(CommLib*, bool v);

// errors
char*    cl_strerror(int code);
void     cl_perror(CommLib* cl);

// requests
int      cl_enquiry(CommLib* cl);
int      cl_read_memory(CommLib* cl, uint16_t addr, uint8_t* buf, size_t sz);
int      cl_write_memory(CommLib* cl, uint16_t addr, uint8_t const* data, size_t sz);
int      cl_status(CommLib* cl, CL_Status* status);
int      cl_cycle(CommLib* cl);
int      cl_init_z80(CommLib* cl);
int      cl_reset_z80(CommLib* cl);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
