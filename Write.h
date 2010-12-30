
#ifndef STATA_WRITE_H
#define STATA_WRITE_H

#include <ruby.h>
#include <inttypes.h>

struct stata_file;
extern int write_stata_file(char * filename, struct stata_file * file);
extern VALUE method_write(VALUE self, VALUE filename, VALUE data);

#endif