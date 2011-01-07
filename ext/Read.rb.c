
#include <math.h>
#include <ruby.h>
#include "Stata.h"
#include "Read.h"

#ifdef HAVE_RUBY_ENCODING_H
# include <ruby/encoding.h>
# define ENCODED_STR_NEW2(str, encoding) \
   ({ \
    VALUE _string = rb_str_new2((const char *)str); \
     int _enc = rb_enc_find_index(encoding); \
     rb_enc_associate_index(_string, _enc); \
     _string; \
   })
#else
# define ENCODED_STR_NEW2(str, encoding) \
   rb_str_new2((const char *)str)
#endif

VALUE method_read(VALUE self, VALUE file)
{
	long i,j;
  
  if (TYPE(file) != T_STRING) rb_raise(rb_eArgError, "filename is not a string, but instead of type '%d' (in C)", TYPE(file));
  
  struct stata_file * f = read_stata_file(rb_string_value_cstr(&file));
  if (f == NULL) rb_raise(rb_eRuntimeError, "Read Error");
  if (f->error) rb_raise(rb_eRuntimeError, "%s", f->error);
  
  
  /* 5.1 Headers */
  VALUE r = rb_hash_new();
  rb_hash_aset(r, ENCODED_STR_NEW2("file_name", "ASCII-8BIT"), ENCODED_STR_NEW2(f->filename, "ASCII-8BIT"));
  rb_hash_aset(r, ENCODED_STR_NEW2("data_label", "ASCII-8BIT"), ENCODED_STR_NEW2(f->data_label, "ASCII-8BIT"));
  rb_hash_aset(r, ENCODED_STR_NEW2("time_stamp", "ASCII-8BIT"), ENCODED_STR_NEW2(f->time_stamp, "ASCII-8BIT"));
  rb_hash_aset(r, ENCODED_STR_NEW2("nvar", "ASCII-8BIT"), INT2NUM(f->nvar));
  rb_hash_aset(r, ENCODED_STR_NEW2("nobs", "ASCII-8BIT"), INT2NUM(f->nobs));
  
  VALUE data = rb_ary_new();
  for (i = 0 ; i < f->nobs ; i++)
  {
    VALUE row = rb_ary_new();
    for (j = 0 ; j < f->nvar ; j++)
    {
      char symbol_name[100];
      sprintf(symbol_name, "%d", f->obs[i].var[j].v_type);
      VALUE var = Qnil;
      sprintf(symbol_name, "dot_");
      
      if (f->obs[i].var[j].v_type == V_STR && f->obs[i].var[j].v_str != NULL)
        var = ENCODED_STR_NEW2(f->obs[i].var[j].v_str, "ASCII-8BIT");
      else if (f->obs[i].var[j].v_type == V_BYTE)
      {
        if (f->obs[i].var[j].v_byte >= -127 && f->obs[i].var[j].v_byte <= 100)
          var = INT2NUM((int)f->obs[i].var[j].v_byte);
        else if (f->obs[i].var[j].v_byte > 100)
        {
          int dot = f->obs[i].var[j].v_byte - 101;
          if (dot == 0) symbol_name[3] = 0;
          else symbol_name[4] = dot+96;
          symbol_name[5] = 0;
          var = rb_str_intern(ENCODED_STR_NEW2(symbol_name, "ASCII-8BIT"));
        }
      }
      else if (f->obs[i].var[j].v_type == V_INT)
      {
        if (f->obs[i].var[j].v_int >= -32767 && f->obs[i].var[j].v_int <= 32740)
          var = INT2NUM((int)f->obs[i].var[j].v_int);
        else if (f->obs[i].var[j].v_int > 32740)
        {
          int dot = f->obs[i].var[j].v_int - 32741;
          if (dot == 0) symbol_name[3] = 0;
          else symbol_name[4] = dot+96;
          symbol_name[5] = 0;
          var = rb_str_intern(ENCODED_STR_NEW2(symbol_name, "ASCII-8BIT"));
        }
      }
      else if (f->obs[i].var[j].v_type == V_LONG)
      {
        if (f->obs[i].var[j].v_long >= -2147483647 && f->obs[i].var[j].v_long <= 2147483620)
          var = LONG2NUM((int)f->obs[i].var[j].v_long);
        else if (f->obs[i].var[j].v_long > 2147483620)
        {
          int dot = f->obs[i].var[j].v_long - 2147483621;
          if (dot == 0) symbol_name[3] = 0;
          else symbol_name[4] = dot+96;
          symbol_name[5] = 0;
          var = rb_str_intern(ENCODED_STR_NEW2(symbol_name, "ASCII-8BIT"));
        }
      }
      else if (f->obs[i].var[j].v_type == V_FLOAT)
      {
        if (f->obs[i].var[j].v_float < pow(2, 127))
        {
          var = rb_float_new(f->obs[i].var[j].v_float);
        }
        else if (f->obs[i].var[j].v_float >= pow(2, 127))
        {
          int dot = (f->obs[i].var[j].v_float - (float)pow(2, 127)) / (float)pow(2, 115);
          if (dot == 0) symbol_name[3] = 0;
          else symbol_name[4] = dot+96;
          symbol_name[5] = 0;
          var = rb_str_intern(ENCODED_STR_NEW2(symbol_name, "ASCII-8BIT"));
        }
      }
      else if (f->obs[i].var[j].v_type == V_DOUBLE)
      {
        if (f->obs[i].var[j].v_double < pow(2, 1023))
        {
          var = rb_dbl2big(f->obs[i].var[j].v_double);
        }
        else if (f->obs[i].var[j].v_double >= pow(2, 1023))
        {
          int dot = (int)((f->obs[i].var[j].v_double - pow(2, 1023)) / pow(2, 1011));
          if (dot == 0) symbol_name[3] = 0;
          else symbol_name[4] = dot+96;
          symbol_name[5] = 0;
          var = rb_str_intern(ENCODED_STR_NEW2(symbol_name, "ASCII-8BIT"));
        }
      }
      
      rb_ary_push(row, var);
    }
    rb_ary_push(data, row);
  }
  rb_hash_aset(r, ENCODED_STR_NEW2("data", "ASCII-8BIT"), data);
  
  
  VALUE fields = rb_ary_new();
  for (i = 0 ; i < f->nvar ; i++)
  {
    VALUE field = rb_hash_new();
    rb_hash_aset(field, ENCODED_STR_NEW2("id", "ASCII-8BIT"), INT2NUM(i+1));
    rb_hash_aset(field, ENCODED_STR_NEW2("type", "ASCII-8BIT"), INT2NUM(f->typlist[i]));
    rb_hash_aset(field, ENCODED_STR_NEW2("name", "ASCII-8BIT"), ENCODED_STR_NEW2(f->varlist[i], "ASCII-8BIT"));
    rb_hash_aset(field, ENCODED_STR_NEW2("format", "ASCII-8BIT"), ENCODED_STR_NEW2(f->fmtlist[i], "ASCII-8BIT"));
    rb_hash_aset(field, ENCODED_STR_NEW2("variable_label", "ASCII-8BIT"), ENCODED_STR_NEW2(f->variable_labels[i], "ASCII-8BIT"));
    rb_hash_aset(field, ENCODED_STR_NEW2("value_label", "ASCII-8BIT"), ENCODED_STR_NEW2(f->lbllist[i], "ASCII-8BIT"));
    rb_hash_aset(field, ENCODED_STR_NEW2("sort", "ASCII-8BIT"), INT2NUM(f->srtlist[i]));
    rb_ary_push(fields, field);
  }
  rb_hash_aset(r, ENCODED_STR_NEW2("fields", "ASCII-8BIT"), fields);
  
  
  VALUE vlt = rb_ary_new();
  for (i = 0 ; i < f->num_vlt ; i++)
  {
    VALUE v = rb_hash_new();
    rb_hash_aset(v, ENCODED_STR_NEW2("name", "ASCII-8BIT"), ENCODED_STR_NEW2(f->vlt[i].name, "ASCII-8BIT"));
    VALUE table = rb_ary_new();
    for (j = 0 ; j < f->vlt[i].n ; j++)
    {
      VALUE row = rb_ary_new();
      rb_ary_push(row, INT2NUM(f->vlt[i].val[j]));
      rb_ary_push(row, ENCODED_STR_NEW2(f->vlt[i].txtbuf + f->vlt[i].off[j], "ASCII-8BIT"));
      
      rb_ary_push(table, row);
    }
    
    rb_hash_aset(v, ENCODED_STR_NEW2("table", "ASCII-8BIT"), table);
    rb_ary_push(vlt, v);
  }
  rb_hash_aset(r, ENCODED_STR_NEW2("value_labels", "ASCII-8BIT"), vlt);
  
  free_stata(f);
  return r;
}
