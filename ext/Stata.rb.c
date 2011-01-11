
#include <ruby.h>
#include "Stata.h"

VALUE method_read(VALUE self, VALUE file);
VALUE method_write(VALUE self, VALUE file);
VALUE method_get_verbose(VALUE self);
VALUE method_set_verbose(VALUE self, VALUE value);

void Init_Stata()
{
  VALUE Stata_module = rb_define_module("Stata");
  rb_define_singleton_method(Stata_module, "read", method_read, 1);
  rb_define_singleton_method(Stata_module, "write", method_write, 2);
  rb_define_singleton_method(Stata_module, "verbose", method_get_verbose, 0);
  rb_define_singleton_method(Stata_module, "verbose=", method_set_verbose, 1);
}

VALUE method_get_verbose(VALUE self)
{
  return verbose ? Qtrue : Qfalse;
}

VALUE method_set_verbose(VALUE self, VALUE value)
{
  verbose = RTEST(value);
  return method_get_verbose(self);
}
