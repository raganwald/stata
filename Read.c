
#include "Read.h"
#include "Stata.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int16_t read_int16_t() { int16_t t=0; fread(&t, sizeof(t), 1, fp); return ((swap_endian_needed==1) ? ((t>>8) | (t<<8)) : t); }
int32_t read_int32_t() { int32_t t=0; fread(&t, sizeof(t), 1, fp); return (swap_endian_needed==1) ? (int32_t)__builtin_bswap32(t) : t; }

uint16_t read_uint16_t() { uint16_t t=0; fread(&t, sizeof(t), 1, fp); return (swap_endian_needed==1) ? (t>>8)|(t<<8) : t; }
uint32_t read_uint32_t() { uint32_t t=0; fread(&t, sizeof(t), 1, fp); return (swap_endian_needed==1) ? __builtin_bswap32(t) : t; }

char * read_string(int length) { char * t = (char*)malloc(length+1); fread(t, length, 1, fp); t[length] = 0; return t; }
char ** read_strings(int num, int length) { char ** t = (char **)malloc(sizeof(char *)*num); int i; for (i = 0 ; i < num ; i++) t[i] = read_string(length); return t; }

float read_float_t() { uint32_t t=0; fread(&t, sizeof(t), 1, fp); if (swap_endian_needed==1) t = __builtin_bswap32(t); return *((float *)(void *)&t); }
double read_double_t() { uint64_t t=0; fread(&t, sizeof(t), 1, fp); if (swap_endian_needed==1) t = __builtin_bswap64(t); return *((double *)(void *)&t); }

struct stata_file * read_stata_file(char * filename)
{
  long i,j;
  
  struct stata_file * f = (struct stata_file *)malloc(sizeof(struct stata_file));
  memset(f, 0, sizeof(struct stata_file));
  f->filename = (char*)malloc(strlen(filename)+1);
  strcpy(f->filename, filename);
  
  fp = fopen(f->filename, "rb");
  if (fp == NULL) { fprintf(stderr, "error opening file \"%s\"\n", f->filename); return f; }
  
  /*fseek(fp, 0 , SEEK_END);
  long lSize = ftell(fp);
  rewind(fp);
  printf("file is %ld bytes long\n", lSize);*/
  
  
  /* 5.1 Header */
  f->ds_format = fgetc(fp);  assert(f->ds_format == 0x72);
  f->byteorder = fgetc(fp);  assert(f->byteorder == 0x01 || f->byteorder == 0x02);
  if (f->byteorder != get_host_endian()) swap_endian_needed = 1;
  f->filetype = fgetc(fp);   assert(f->filetype == 0x01);
  f->unused = fgetc(fp);     assert(f->unused == 0x00);
  f->nvar = read_uint16_t(); assert(f->nvar > 1);
  f->nobs = read_uint32_t(); assert(f->nobs > 1);
  fread(&f->data_label, sizeof(f->data_label), 1, fp);
  fread(&f->time_stamp, sizeof(f->time_stamp), 1, fp);
  printf("read file \"%s\"\n", f->filename);
  
  
  /* 5.2 Descriptors */
  f->typlist = (uint8_t *)malloc(f->nvar);
  fread(f->typlist, 1, f->nvar, fp);
  f->varlist = read_strings(f->nvar, 33);
  f->srtlist = (uint16_t *)malloc(sizeof(uint16_t)*(f->nvar+1));
  for (i = 0 ; i <= f->nvar ; i++) f->srtlist[i] = read_uint16_t();
  f->fmtlist = read_strings(f->nvar, 49);
  f->lbllist = read_strings(f->nvar, 33);
  
  
  /* 5.3 Variable Labels */
  f->variable_labels = read_strings(f->nvar, 81);
  
  
  /* 5.4 Expansion Fields */
  uint8_t data_type;
  uint32_t len;
  do {
    data_type = fgetc(fp);
    assert(fread(&len, 4, 1, fp)==1);
    if (len > 0) for (i = 0 ; i < len ; i++) fgetc(fp);
  } while(data_type != 0 || len != 0 || feof(fp));
  /*printf ("read %d bytes of expansion fields\n", count);*/
  
  
  /* 5.5 Data */
  /*printf("  read 5.5 Data (%dx%d)\n", f->nobs, f->nvar);*/
  f->obs = (struct stata_obs *)malloc(sizeof(struct stata_obs)*f->nobs);
  for (j = 0 ; j < f->nobs ; j++)
  {
    f->obs[j].var = (struct stata_var *)malloc(sizeof(struct stata_var)*f->nvar);
    for (i = 0 ; i < f->nvar ; i++)
    {
      struct stata_var * var = &f->obs[j].var[i];
      memset(var, 0, sizeof(struct stata_var));
      
      if (f->typlist[i] != 0 && 
                f->typlist[i] < 245) { var->v_type = V_STR;    var->v_str = read_string(f->typlist[i]); assert(!feof(fp)); }
      else if (f->typlist[i] == 251) { var->v_type = V_BYTE;   assert(fread(&var->v_byte, sizeof(var->v_byte), 1, fp)==1); assert(!feof(fp)); }
      else if (f->typlist[i] == 252) { var->v_type = V_INT;    var->v_int = read_int16_t();     assert(!feof(fp)); }
      else if (f->typlist[i] == 253) { var->v_type = V_LONG;   var->v_long = read_int32_t();    assert(!feof(fp)); }
      else if (f->typlist[i] == 254) { var->v_type = V_FLOAT;  var->v_float = read_float_t();   assert(!feof(fp)); }
      else if (f->typlist[i] == 255) { var->v_type = V_DOUBLE; var->v_double = read_double_t(); assert(!feof(fp)); }
      else fprintf(stderr, "error.\n");
      
      if (ferror(fp)) perror("error occurred");
      if (feof(fp)) { fprintf(stderr, "eof error at var %ld (error:%d)\n", i, ferror(fp)); break; }
    }
    if (feof(fp)) { fprintf(stderr, "eof error at obs %ld (error:%d)\n", j, ferror(fp)); exit(1); }
  }
  
  
  /* 5.6 Value labels */
  if (!feof(fp))
  {
    while (!feof(fp))
    {
      int32_t len = read_int32_t();
      if (feof(fp) || len == 0) break;
      
      f->num_vlt++;
      f->vlt = (struct stata_vlt *)realloc(f->vlt, sizeof(struct stata_vlt)*f->num_vlt);
      struct stata_vlt * vlt = &f->vlt[f->num_vlt-1];
      
      assert(fread(vlt->name, 33, 1, fp)==1);
      fgetc(fp); fgetc(fp); fgetc(fp); /* padding */
      
      vlt->n = read_int32_t();
      vlt->txtlen = read_int32_t(); assert(vlt->txtlen < 32000);
      vlt->off = (int32_t*)malloc(sizeof(int32_t)*vlt->n);
      for (i = 0 ; i < vlt->n ; i++) vlt->off[i] = read_int32_t();
      vlt->val = (int32_t*)malloc(sizeof(int32_t)*vlt->n);
      for (i = 0 ; i < vlt->n ; i++) vlt->val[i] = read_int32_t();
      vlt->txtbuf = (char*)malloc(vlt->txtlen);
      fread(vlt->txtbuf, vlt->txtlen, 1, fp);
    }
  }
  /*printf("  read 5.6 Value labels (%d)\n", f->num_vlt);*/

  
  fclose(fp);
  return f;
}
