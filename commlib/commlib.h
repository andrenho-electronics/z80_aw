#ifndef COMMLIB_H_
#define COMMLIB_H_

#include <stdint.h>

typedef struct CommLib CommLib;

// initialization
CommLib* cl_init(const char* comfile, int speed);
void     cl_free(CommLib* cl);

// errors
char*    cl_strerror(int code);

// requests
int      cl_enquiry(CommLib* cl);

#endif
