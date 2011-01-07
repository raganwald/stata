
#include "Write.h"
#include "Stata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct stata_file;

int write_stata_file(char * filename, struct stata_file * f)
{
  printf("write file '%s'\n", filename);
  
  long i,j;
  
  if (f == NULL) return 0;
  
  fp = fopen(filename, "wb");
  if (fp == NULL) return set_error(f, "error opening file");
  
  if (f->nvar <= 0) return set_error(f, "nvar should be more then 0"); 
  if (f->nobs <= 0) return set_error(f, "nobs should be more then 0"); 
  
  /* 5.1 Headers */
  char header[4] = {0x72, get_host_endian(), 0x01, 0x00};
  if (fwrite(header, 4, 1, fp) != 1) return set_error(f, "fwrite to file failed");
  if (fwrite(&f->nvar, sizeof(f->nvar), 1, fp) != 1) return set_error(f, "fwrite to file failed");
  if (fwrite(&f->nobs, sizeof(f->nobs), 1, fp) != 1) return set_error(f, "fwrite to file failed");
  if (fwrite(f->data_label, sizeof(f->data_label), 1, fp) != 1) return set_error(f, "fwrite to file failed");
  f->time_stamp[17] = 0;
  if (fwrite(f->time_stamp, sizeof(f->time_stamp), 1, fp) != 1) return set_error(f, "fwrite to file failed");
  
  
  /* 5.2 Descriptors */
  if (fwrite(f->typlist, 1, f->nvar, fp) != f->nvar) return set_error(f, "fwrite to file failed");
  for (i = 0 ; i < f->nvar ; i++) if (fwrite(f->varlist[i], 33, 1, fp) != 1) return set_error(f, "fwrite to file failed");
  if (fwrite(f->srtlist, 2, f->nvar+1, fp) != (unsigned int)f->nvar+1) return set_error(f, "fwrite to file failed");
  for (i = 0 ; i < f->nvar ; i++) if (fwrite(f->fmtlist[i], 49, 1, fp) != 1) return set_error(f, "fwrite to file failed");
  for (i = 0 ; i < f->nvar ; i++) if (fwrite(f->lbllist[i], 33, 1, fp) != 1) return set_error(f, "fwrite to file failed");
  
  
  /* 5.3 Variable Labels */
  for (i = 0 ; i < f->nvar ; i++) if (fwrite(f->variable_labels[i], 81, 1, fp) != 1) return set_error(f, "fwrite to file failed");
  
  
  /* 5.4 Expansion Fields */
  char zeros[5] = {0,0,0,0,0};
  if (fwrite(zeros, 5, 1, fp) != 1) return set_error(f, "fwrite to file failed");
  
  
  /* 5.5 Data */
  /*printf("  write 5.5 Data (%dx%d)\n", f->nobs, f->nvar);*/
  for (j = 0 ; j < f->nobs ; j++)
  {
    for (i = 0 ; i < f->nvar ; i++)
    {
      struct stata_var * var = &f->obs[j].var[i];
      if (f->typlist[i] != 0 && f->typlist[i] < 245) { if (fwrite(var->v_str, f->typlist[i], 1, fp) != 1) return set_error(f, "fwrite to file failed"); }
      else if (f->typlist[i] == 251) { if (fwrite(&var->v_byte, sizeof(var->v_byte), 1, fp) != 1) return set_error(f, "fwrite to file failed"); }
      else if (f->typlist[i] == 252) { if (fwrite(&var->v_int, sizeof(var->v_int), 1, fp) != 1) return set_error(f, "fwrite to file failed"); }
      else if (f->typlist[i] == 253) { if (fwrite(&var->v_long, sizeof(var->v_long), 1, fp) != 1) return set_error(f, "fwrite to file failed"); }
      else if (f->typlist[i] == 254) { if (fwrite(&var->v_float, sizeof(var->v_float), 1, fp) != 1) return set_error(f, "fwrite to file failed"); }
      else if (f->typlist[i] == 255) { if (fwrite(&var->v_double, sizeof(var->v_double), 1, fp) != 1) return set_error(f, "fwrite to file failed"); }
    }
  }
  
  
  /* 5.6 Value Labels */
  /*printf("  write 5.6 Value Labels (%d)\n", f->num_vlt);*/
  for (i = 0 ; i < f->num_vlt ; i++)
  {
    struct stata_vlt * vlt = &f->vlt[i];
    uint32_t len = 4 + 4 + 4*vlt->n + 4*vlt->n + vlt->txtlen;
    if (fwrite(&len, sizeof(len), 1, fp) != 1) return set_error(f, "fwrite to file failed");
    if (fwrite(vlt->name, 33, 1, fp) != 1) return set_error(f, "fwrite to file failed");
    if (fwrite(zeros, 3, 1, fp) != 1) return set_error(f, "fwrite to file failed");
    if (fwrite(&vlt->n, sizeof(vlt->n), 1, fp) != 1) return set_error(f, "fwrite to file failed");
    uint32_t txtlen = 0;
    for (j = 0 ; j < vlt->n ; j++)
      txtlen += (int)strlen(vlt->txtbuf + vlt->off[j]) + 1;
    
    if (fwrite(&txtlen, sizeof(txtlen), 1, fp) != 1) return set_error(f, "fwrite to file failed");
    if (fwrite(vlt->off, sizeof(uint32_t), vlt->n, fp) != (unsigned int)vlt->n) return set_error(f, "fwrite to file failed");
    if (fwrite(vlt->val, sizeof(uint32_t), vlt->n, fp) != (unsigned int)vlt->n) return set_error(f, "fwrite to file failed");
    if (fwrite(vlt->txtbuf, txtlen, 1, fp) != 1) return set_error(f, "fwrite to file failed");
  }
  
  fclose(fp);
  return 0;
}























