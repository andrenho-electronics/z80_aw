#ifndef COMMLIB_H_
#define COMMLIB_H_

#include <stdint.h>

typedef struct CommLib CommLib;

CommLib* cl_init(const char* comfile);
void     cl_free(CommLib* cl);

int      cl_enquiry();

#endif
