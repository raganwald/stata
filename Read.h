
#ifndef STATA_READ_H
#define STATA_READ_H

#include <ruby.h>
#include <inttypes.h>

extern int16_t read_int16_t();
extern int32_t read_int32_t();

extern uint16_t read_uint16_t();
extern uint32_t read_uint32_t();
extern uint64_t read_uint64_t();

extern char * read_string(int length);
extern char ** read_strings(int num, int length);

extern float read_float_t();
extern float read_double_t();

struct stata_file;
extern struct stata_file * read_stata_file(char * filename);
extern VALUE method_read(VALUE self, VALUE file);

#endif