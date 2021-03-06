
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ruby.h>
#include "Stata.h"
#include "Write.h"

#ifndef RUBY_19
#ifndef RFLOAT_VALUE
#define RFLOAT_VALUE(v) (RFLOAT(v)->value)
#endif
#ifndef RARRAY_LEN
#define RARRAY_LEN(v) (RARRAY(v)->len)
#endif
#ifndef RARRAY_PTR
#define RARRAY_PTR(v) (RARRAY(v)->ptr)
#endif
#endif

int populate_fields_from_ruby_index = 0;
VALUE populate_fields_from_ruby(VALUE field, struct stata_file * f)
{
  VALUE v;
  
  v = rb_hash_aref(field, rb_str_new2("type"));
  if (TYPE(v) != T_FIXNUM) rb_raise(rb_eArgError, "field type is not provided or is not a Fixnum");
  f->typlist[populate_fields_from_ruby_index] = NUM2INT(v);
  
  v = rb_hash_aref(field, rb_str_new2("name"));
  if (TYPE(v) != T_STRING) rb_raise(rb_eArgError, "field name is not provided or is not a Fixnum");
  f->varlist[populate_fields_from_ruby_index] = (char*)malloc(33);
  strncpy(f->varlist[populate_fields_from_ruby_index], rb_string_value_cstr(&v), 33);
  
  v = rb_hash_aref(field, rb_str_new2("sort"));
  if (TYPE(v) != T_FIXNUM) rb_raise(rb_eArgError, "field sort is not provided or is not a Fixnum");
  f->srtlist[populate_fields_from_ruby_index] = NUM2INT(v);
  
  v = rb_hash_aref(field, rb_str_new2("format"));
  if (TYPE(v) != T_STRING) rb_raise(rb_eArgError, "field format is not provided or is not a Fixnum");
  f->fmtlist[populate_fields_from_ruby_index] = (char*)malloc(49);
  strncpy(f->fmtlist[populate_fields_from_ruby_index], rb_string_value_cstr(&v), 49);
  
  v = rb_hash_aref(field, rb_str_new2("value_label"));
  if (TYPE(v) != T_STRING) rb_raise(rb_eArgError, "field value_label is not provided or is not a String");
  f->lbllist[populate_fields_from_ruby_index] = (char*)malloc(33);
  strncpy(f->lbllist[populate_fields_from_ruby_index], rb_string_value_cstr(&v), 33);
  
  v = rb_hash_aref(field, rb_str_new2("variable_label"));
  if (TYPE(v) != T_STRING) rb_raise(rb_eArgError, "field variable_label is not provided or is not a String");
  f->variable_labels[populate_fields_from_ruby_index] = (char*)malloc(81);
  strncpy(f->variable_labels[populate_fields_from_ruby_index], rb_string_value_cstr(&v), 81);
  
  populate_fields_from_ruby_index++;
}

int populate_data_from_ruby_index = 0;
VALUE populate_data_from_ruby(VALUE row, struct stata_file * f)
{
  VALUE v;
  int j = 0;//, i = populate_data_from_ruby_index;
  for (j = 0 ; j < f->nvar ; j++)
  {
    v = rb_ary_entry(row, j);
    struct stata_var * var = &f->obs[populate_data_from_ruby_index].var[j];
    
    if (f->typlist[j] == 255) var->v_type = V_DOUBLE;
    else if (f->typlist[j] == 254) var->v_type = V_FLOAT;
    else if (f->typlist[j] == 253) var->v_type = V_LONG;
    else if (f->typlist[j] == 252) var->v_type = V_INT;
    else if (f->typlist[j] == 251) var->v_type = V_BYTE;
    else if (f->typlist[j] <= 244) var->v_type = f->typlist[j];
    else rb_raise(rb_eArgError, "type is %d", f->typlist[j]);
    
    switch (TYPE(v)) {
      case T_SYMBOL:
        v = rb_str_new2(rb_id2name(SYM2ID(v)));
        const char * symbol_name = RSTRING_PTR(v);
        
        int dot = 0;
        if (strlen(symbol_name) == 5) dot = symbol_name[4] - 96;
        if (dot < 0 || dot > 26) { rb_raise(rb_eArgError, "INVALID SYMBOL '%s'", symbol_name); continue; }
        
        if (f->typlist[j] == 255) var->v_double = pow(2, 1023) + dot*pow(2, 1011);
        else if (f->typlist[j] == 254) var->v_float = (float)pow(2, 127) + dot*(float)pow(2, 115);
        else if (f->typlist[j] == 253) var->v_long = 2147483621 + dot;
        else if (f->typlist[j] == 252) var->v_int = 32741 + dot;
        else if (f->typlist[j] == 251) var->v_byte = 101 + dot;
        else rb_raise(rb_eArgError, "invalid typlist '%d' %d", f->typlist[j], TYPE(v));
      break;
      case T_BIGNUM:
      case T_FIXNUM:
      case T_FLOAT:
        if (f->typlist[j] == 255) var->v_double = rb_num2dbl(v);
        else if (f->typlist[j] == 254) var->v_float = (float)rb_num2dbl(v);
        else if (f->typlist[j] == 253) var->v_long = (int32_t)FIX2LONG(v);
        else if (f->typlist[j] == 252) var->v_int = FIX2LONG(v);
        else if (f->typlist[j] == 251) var->v_byte = FIX2LONG(v);
        else rb_raise(rb_eArgError, "invalid typlist '%d' %d %f", f->typlist[j], TYPE(v), RFLOAT_VALUE(v));
      break;
      case T_STRING:
        var->v_type = f->typlist[j];
        var->v_str = (char*)malloc(f->typlist[j]+1);
        strncpy(var->v_str, RSTRING_PTR(v), f->typlist[j]+1);
      break;
      case T_NIL:
        rb_raise(rb_eArgError, "nil value submitted");
      break;
      default:
        rb_raise(rb_eArgError, "unsupported ruby type: %d", TYPE(v));
      break;
    }
  }
  populate_data_from_ruby_index++;
}

int populate_value_labels_from_ruby_index = 0;
VALUE populate_value_labels_from_ruby(VALUE r_vlt, struct stata_file * f)
{
  VALUE v;
  
  f->num_vlt++;
  f->vlt = (struct stata_vlt *)realloc(f->vlt, sizeof(struct stata_vlt)*f->num_vlt);
  struct stata_vlt * vlt = &f->vlt[f->num_vlt-1];
  
  if (TYPE(r_vlt) != T_HASH) rb_raise(rb_eArgError, "Value label table should be a Hash");
  
  v = rb_hash_aref(r_vlt, rb_str_new2("name"));
  if (TYPE(v) != T_STRING) rb_raise(rb_eArgError, "Value label table name isn't provided or isn't a String");
  strncpy(vlt->name, RSTRING_PTR(v), 33);
  
  v = rb_hash_aref(r_vlt, rb_str_new2("table"));
  if (TYPE(v) != T_ARRAY) rb_raise(rb_eArgError, "Value label table name isn't provided or isn't an Array");
  
  vlt->n = (int32_t)RARRAY_LEN(v);
  vlt->txtlen = 0;
  vlt->off = (uint32_t*)malloc(sizeof(uint32_t)*vlt->n);
  vlt->val = (uint32_t*)malloc(sizeof(uint32_t)*vlt->n);
  vlt->txtbuf = NULL;
  
  int i;
  for (i = 0 ; i < RARRAY_LEN(v) ; i++)
  {
    VALUE r = rb_ary_entry(v, i);
    if (TYPE(r) != T_ARRAY) rb_raise(rb_eArgError, "value label contains a row which isn't an Array");
    if (TYPE(rb_ary_entry(r, 0)) != T_FIXNUM) rb_raise(rb_eArgError, "value label contains a value which isn't a Fixnum");
    if (TYPE(rb_ary_entry(r, 1)) != T_STRING) rb_raise(rb_eArgError, "value label contains a label which isn't a String");
    char * txt = RSTRING_PTR(rb_ary_entry(r, 1));
    vlt->txtlen += (int32_t)strlen(txt)+1;
  }
  vlt->txtbuf = (char*)malloc(vlt->txtlen);
  
  vlt->txtlen = 0;
  for (i = 0 ; i < RARRAY_LEN(v) ; i++)
  {
    VALUE r = rb_ary_entry(v, i);
    vlt->val[i] = NUM2INT(rb_ary_entry(r, 0));
    char * txt = RSTRING_PTR(rb_ary_entry(r, 1));
    vlt->txtlen += (int32_t)strlen(txt)+1;
    
    vlt->off[i] = vlt->txtlen-((int32_t)strlen(txt)+1);
    memcpy(vlt->txtbuf+vlt->off[i], txt, strlen(txt)+1);
  }
  
  populate_value_labels_from_ruby_index++;
}

VALUE method_write(VALUE self, VALUE filename, VALUE data)
{
  VALUE v;
  if (TYPE(data) != T_HASH) rb_raise(rb_eArgError, "Content to be written should be a hash");
  if (TYPE(filename) != T_STRING) rb_raise(rb_eArgError, "Filename for writing is not a string");
  
  if (rb_hash_aref(data, rb_str_new2("nvar")) == Qnil) rb_raise(rb_eArgError, "nvar is required");
  if (rb_hash_aref(data, rb_str_new2("nobs")) == Qnil) rb_raise(rb_eArgError, "nobs is required");
  if (rb_hash_aref(data, rb_str_new2("fields")) == Qnil) rb_raise(rb_eArgError, "no fields provided");
  if (rb_hash_aref(data, rb_str_new2("data")) == Qnil) rb_raise(rb_eArgError, "no data provided");
  
  struct stata_file * f = (struct stata_file *)malloc(sizeof(struct stata_file));
  if (f == NULL) rb_raise(rb_eArgError, "Could not allocate memory for the stata file");
  memset(f, 0, sizeof(struct stata_file));
  
  
  /* 5.1 Headers */
  v = rb_hash_aref(data, rb_str_new2("nvar"));
  if (TYPE(v) != T_FIXNUM) rb_raise(rb_eArgError, "nvar is not provided or is not a fixnum");
  f->nvar = NUM2UINT(v);
  
  v = rb_hash_aref(data, rb_str_new2("nobs"));
  if (TYPE(v) != T_FIXNUM) rb_raise(rb_eArgError, "nobs is not provided or is not a fixnum");
  f->nobs = NUM2UINT(v);
  
  v = rb_hash_aref(data, rb_str_new2("data_label"));
  if (TYPE(v) != T_STRING) rb_raise(rb_eArgError, "data_label is not provided or is not a string");
  strncpy(f->data_label, rb_string_value_cstr(&v), sizeof(f->data_label));
  
  v = rb_hash_aref(data, rb_str_new2("time_stamp"));
  if (TYPE(v) != T_STRING) rb_raise(rb_eArgError, "time_stamp is not provided or is not a string");
  strncpy(f->time_stamp, rb_string_value_cstr(&v), sizeof(f->time_stamp));
  
  
  /* 5.2 and 5.3, Descriptors and Variable Labels */
  f->typlist = (uint8_t *)malloc(f->nvar);
  if (f->typlist == NULL) rb_raise(rb_eRuntimeError, "Could not allocate more memory");
  f->varlist = (char **)malloc(sizeof(char *)*f->nvar);
  if (f->varlist == NULL) rb_raise(rb_eRuntimeError, "Could not allocate more memory");
  f->srtlist = (uint16_t *)malloc(sizeof(uint16_t)*(f->nvar+1));
  if (f->srtlist == NULL) rb_raise(rb_eRuntimeError, "Could not allocate more memory");
  f->fmtlist = (char **)malloc(sizeof(char *)*f->nvar);
  if (f->fmtlist == NULL) rb_raise(rb_eRuntimeError, "Could not allocate more memory");
  f->lbllist = (char **)malloc(sizeof(char *)*f->nvar);
  if (f->lbllist == NULL) rb_raise(rb_eRuntimeError, "Could not allocate more memory");
  f->variable_labels = (char **)malloc(sizeof(char *)*f->nvar);
  if (f->variable_labels == NULL) rb_raise(rb_eRuntimeError, "Could not allocate more memory");
  
  v = rb_hash_aref(data, rb_str_new2("fields"));
  if (TYPE(v) != T_ARRAY) rb_raise(rb_eArgError, "fields are not provided or are not an array");
  
  populate_fields_from_ruby_index = 0;
  rb_iterate(rb_each, v, populate_fields_from_ruby, (VALUE)f);
  
  
  /* 5.3 Expansion Fields - nothing comes from ruby */
  
  
  /* 5.4 Data */
  long i, j;
  f->obs = (struct stata_obs *)malloc(sizeof(struct stata_obs)*f->nobs);
  for (j = 0 ; j < f->nobs ; j++)
  {
    f->obs[j].var = (struct stata_var *)malloc(sizeof(struct stata_var)*f->nvar);
    for (i = 0 ; i < f->nvar ; i++)
    {
      struct stata_var * var = &f->obs[j].var[i];
      memset(var, 0, sizeof(struct stata_var));
    }
  }
  v = rb_hash_aref(data, rb_str_new2("data"));
  if (TYPE(v) != T_ARRAY) rb_raise(rb_eArgError, "data is not provided or is not an array");
  
  populate_data_from_ruby_index = 0;
  rb_iterate(rb_each, v, populate_data_from_ruby, (VALUE)f);
  
  
  /* 5.5 Value Label Tables */
  v = rb_hash_aref(data, rb_str_new2("value_labels"));
  if (TYPE(v) != T_ARRAY) rb_raise(rb_eArgError, "value labels are not provided or are not an array");
  
  populate_value_labels_from_ruby_index = 0;
  rb_iterate(rb_each, v, populate_value_labels_from_ruby, (VALUE)f);
  
  write_stata_file(RSTRING_PTR(filename), f);
  
  if (f->error) rb_raise(rb_eRuntimeError, "%s", f->error);
  
  free_stata(f);
  
  return INT2NUM(1);
}
