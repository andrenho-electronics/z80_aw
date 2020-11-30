#ifndef COMMLIB_H_
#define COMMLIB_H_

#include <stdint.h>

typedef struct CommLib CommLib;

// initialization
CommLib* cl_init(const char* comfile, int speed);
void     cl_free(CommLib* cl);

// errors
char*    cl_strerror(int code);
void     cl_perror(CommLib* cl);

// requests
int      cl_enquiry(CommLib* cl);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
