
#ifndef STATA_H
#define STATA_H

#include <stdio.h>
#include <inttypes.h>

extern FILE * fp;
extern int swap_endian_needed;

enum stata_type {
  V_INVALID, V_STR, V_BYTE, V_INT, V_LONG, V_FLOAT, V_DOUBLE
};

struct stata_var {
  enum stata_type v_type;
  union {
    char * v_str;
    int8_t v_byte;
    int16_t v_int;
    int32_t v_long;
    float v_float;
    double v_double;
  };
};

struct stata_obs {
  struct stata_var * var;
};

struct stata_vlt {
  char name[33];
  int32_t n;
  int32_t txtlen;
  int32_t * off;
  int32_t * val;
  char * txtbuf;
};

struct stata_file {
  char * filename;
  
  int8_t ds_format;
  int8_t byteorder;
  int8_t filetype;
  int8_t unused;
  uint16_t nvar;
  uint32_t nobs;
  char data_label[81];
  char time_stamp[18];
  
  
  uint8_t * typlist;
  char ** varlist;
  uint16_t * srtlist;
  char ** fmtlist;
  char ** lbllist;
  char ** variable_labels;
  
  struct stata_obs * obs;
  
  int num_vlt;
  struct stata_vlt * vlt;
};

extern char get_host_endian();
extern void free_stata(struct stata_file * f);

#endif
