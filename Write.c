
#include "Write.h"
#include "Stata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

struct stata_file;

int write_stata_file(char * filename, struct stata_file * f)
{
  printf("write file \"%s\"\n", filename);
  int i,j;
  
  fp = fopen(filename, "wb");
  if (fp == NULL) { fprintf(stderr, "error opening file \"%s\"\n", filename); return 0; }
  
  
  /* 5.1 Headers */
  char header[4] = {0x72, get_host_endian(), 0x01, 0x00};
  fwrite(header, 4, 1, fp);
  fwrite(&f->nvar, sizeof(f->nvar), 1, fp);
  fwrite(&f->nobs, sizeof(f->nobs), 1, fp);
  fwrite(f->data_label, sizeof(f->data_label), 1, fp);
  assert(f->time_stamp[17] == 0x00);
  fwrite(f->time_stamp, sizeof(f->time_stamp), 1, fp);
  
  
  /* 5.2 Descriptors */
  fwrite(f->typlist, 1, f->nvar, fp);
  for (i = 0 ; i < f->nvar ; i++) fwrite(f->varlist[i], 33, 1, fp);
  fwrite(f->srtlist, 2, f->nvar+1, fp);
  for (i = 0 ; i < f->nvar ; i++) fwrite(f->fmtlist[i], 49, 1, fp);
  for (i = 0 ; i < f->nvar ; i++) fwrite(f->lbllist[i], 33, 1, fp);
  
  
  /* 5.3 Variable Labels */
  for (i = 0 ; i < f->nvar ; i++) fwrite(f->variable_labels[i], 81, 1, fp);
  
  
  /* 5.4 Expansion Fields */
  char zeros[5] = {0,0,0,0,0};
  fwrite(zeros, 5, 1, fp);
  
  
  /* 5.5 Data */
  int out = 0;
  int count = 0;
  //printf("  write 5.5 Data (%dx%d)\n", f->nobs, f->nvar);
  for (j = 0 ; j < f->nobs ; j++)
  {
    for (i = 0 ; i < f->nvar ; i++)
    {
      struct stata_var * var = &f->obs[j].var[i];
      if (f->typlist[i] != 0 && f->typlist[i] < 245) assert(fwrite(var->v_str, f->typlist[i], 1, fp)==1);
      else if (f->typlist[i] == 251) assert(fwrite(&var->v_byte, sizeof(var->v_byte), 1, fp)==1);
      else if (f->typlist[i] == 252) assert(fwrite(&var->v_int, sizeof(var->v_int), 1, fp)==1);
      else if (f->typlist[i] == 253) assert(fwrite(&var->v_long, sizeof(var->v_long), 1, fp)==1);
      else if (f->typlist[i] == 254) assert(fwrite(&var->v_float, sizeof(var->v_float), 1, fp)==1);
      else if (f->typlist[i] == 255) assert(fwrite(&var->v_double, sizeof(var->v_double), 1, fp)==1);
    }
  }
  
  
  /* 5.6 Value Labels */
  //printf("  write 5.6 Value Labels (%d)\n", f->num_vlt);
  for (i = 0 ; i < f->num_vlt ; i++)
  {
    struct stata_vlt * vlt = &f->vlt[i];
    uint32_t len = 4 + 4 + 4*vlt->n + 4*vlt->n + vlt->txtlen;
    assert(fwrite(&len, sizeof(len), 1, fp)==1);
    assert(fwrite(vlt->name, 33, 1, fp)==1);
    assert(fwrite(zeros, 3, 1, fp)==1);
    assert(fwrite(&vlt->n, sizeof(vlt->n), 1, fp)==1);
    uint32_t txtlen = 0;
    for (j = 0 ; j < vlt->n ; j++)
      txtlen += strlen(vlt->txtbuf + vlt->off[j]) + 1;
    
    assert(fwrite(&txtlen, sizeof(txtlen), 1, fp)==1);
    assert(fwrite(vlt->off, sizeof(uint32_t), vlt->n, fp)==vlt->n);
    assert(fwrite(vlt->val, sizeof(uint32_t), vlt->n, fp)==vlt->n);
    assert(fwrite(vlt->txtbuf, txtlen, 1, fp)==1);
  }
  
  fclose(fp);
  return 0;
}























