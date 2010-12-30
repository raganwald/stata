
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <ruby.h>

#include "Stata.h"
#include "Read.h"
#include "Write.h"

FILE *fp;
int swap_endian_needed = 0;

char get_host_endian()
{
  int i = 1;
  char *p = (char *) &i;
  return (p[0] == 1) ? 0x02 : 0x01;
}

void Init_Stata()
{
  VALUE Stata_module = rb_define_module("Stata");
  rb_define_singleton_method(Stata_module, "read", method_read, 1);
  rb_define_singleton_method(Stata_module, "write", method_write, 2);
}

void free_stata(struct stata_file * f)
{
  return;
  int i;
  free(f->filename);
  for (i = 0 ; i < f->nvar ; i++) free(f->varlist[i]);
  for (i = 0 ; i < f->nvar ; i++) free(f->fmtlist[i]);
  for (i = 0 ; i < f->nvar ; i++) free(f->lbllist[i]);
  for (i = 0 ; i < f->nvar ; i++) free(f->variable_labels[i]);
  free(f->typlist);
  free(f->varlist);
  free(f->srtlist);
  free(f->fmtlist);
  free(f->lbllist);
  free(f->variable_labels);
  
  for (i = 0 ; i < f->nobs ; i++)
  {
    
  }
  free(f->obs);
  
  for (i = 0 ; i < f->num_vlt ; i++)
  {
    free(f->vlt[i].off);
    free(f->vlt[i].val);
    free(f->vlt[i].txtbuf);
  }
  free(f->vlt);
  
  free(f);
}

int main(int argc, char *argv[])
{
  if (argc != 2) { fprintf(stderr, "usage:\n ./read [file]\n"); return(0); }
  struct stata_file * f = read_stata_file(argv[1]);
  write_stata_file("out.dta", f);
  struct stata_file * f1 = read_stata_file(argv[1]);
  write_stata_file("out.dta", f1);
}