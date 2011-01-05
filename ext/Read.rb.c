
#include <assert.h>
#include <math.h>
#include <ruby.h>
#include "Stata.h"
#include "Read.h"

VALUE method_read(VALUE self, VALUE file)
{
	long i,j;
  
  assert(TYPE(file) == T_STRING);
  
  struct stata_file * f = read_stata_file(rb_string_value_cstr(&file));
  
  /* 5.1 Headers */
  VALUE r = rb_hash_new();
  rb_hash_aset(r, rb_str_new2("file_name"), rb_str_new2(f->filename));
  rb_hash_aset(r, rb_str_new2("data_label"), rb_str_new2(f->data_label));
  rb_hash_aset(r, rb_str_new2("time_stamp"), rb_str_new2(f->time_stamp));
  rb_hash_aset(r, rb_str_new2("nvar"), INT2NUM(f->nvar));
  rb_hash_aset(r, rb_str_new2("nobs"), INT2NUM(f->nobs));
  
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
        var = rb_str_new2(f->obs[i].var[j].v_str);
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
          var = rb_str_intern(rb_str_new2(symbol_name));
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
          var = rb_str_intern(rb_str_new2(symbol_name));
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
          var = rb_str_intern(rb_str_new2(symbol_name));
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
          var = rb_str_intern(rb_str_new2(symbol_name));
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
          var = rb_str_intern(rb_str_new2(symbol_name));
        }
      }
      
      rb_ary_push(row, var);
    }
    rb_ary_push(data, row);
  }
  rb_hash_aset(r, rb_str_new2("data"), data);
  
  
  VALUE fields = rb_ary_new();
  for (i = 0 ; i < f->nvar ; i++)
  {
    VALUE field = rb_hash_new();
    rb_hash_aset(field, rb_str_new2("id"), INT2NUM(i+1));
    rb_hash_aset(field, rb_str_new2("type"), INT2NUM(f->typlist[i]));
    rb_hash_aset(field, rb_str_new2("name"), rb_str_new2(f->varlist[i]));
    rb_hash_aset(field, rb_str_new2("format"), rb_str_new2(f->fmtlist[i]));
    rb_hash_aset(field, rb_str_new2("variable_label"), rb_str_new2(f->variable_labels[i]));
    rb_hash_aset(field, rb_str_new2("value_label"), rb_str_new2(f->lbllist[i]));
    rb_hash_aset(field, rb_str_new2("sort"), INT2NUM(f->srtlist[i]));
    rb_ary_push(fields, field);
  }
  rb_hash_aset(r, rb_str_new2("fields"), fields);
  
  
  VALUE vlt = rb_ary_new();
  for (i = 0 ; i < f->num_vlt ; i++)
  {
    VALUE v = rb_hash_new();
    rb_hash_aset(v, rb_str_new2("name"), rb_str_new2(f->vlt[i].name));
    VALUE table = rb_ary_new();
    for (j = 0 ; j < f->vlt[i].n ; j++)
    {
      VALUE row = rb_ary_new();
      rb_ary_push(row, INT2NUM(f->vlt[i].val[j]));
      rb_ary_push(row, rb_str_new2(f->vlt[i].txtbuf + f->vlt[i].off[j]));
      
      rb_ary_push(table, row);
    }
    
    rb_hash_aset(v, rb_str_new2("table"), table);
    rb_ary_push(vlt, v);
  }
  rb_hash_aset(r, rb_str_new2("value_labels"), vlt);
  
  free_stata(f);
  return r;
}