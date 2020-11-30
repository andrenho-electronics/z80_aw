#ifndef COMMLIB_H_
#define COMMLIB_H_

typedef struct CommLib CommLib;

CommLib* cl_init(const char* comfile);
void     cl_destroy(CommLib* cl);

#endif
