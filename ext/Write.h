
#ifndef STATA_WRITE_H
#define STATA_WRITE_H

#include <inttypes.h>

struct stata_file;
extern int write_stata_file(char * filename, struct stata_file * file);

#endif