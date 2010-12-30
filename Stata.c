
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

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

void free_stata(struct stata_file * f)
{
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
  
  char src_file[100];
  char dst_file[100];
  sprintf(src_file, "%s.dta", argv[1]);
  sprintf(dst_file, "%s_resave.dta", argv[1]);
  
  struct stata_file * f = read_stata_file(src_file);
  write_stata_file(dst_file, f);
  return 0;
}
