
#include <ruby.h>

VALUE method_read(VALUE self, VALUE file);
VALUE method_write(VALUE self, VALUE file);

void Init_Stata()
{
  VALUE Stata_module = rb_define_module("Stata");
  rb_define_singleton_method(Stata_module, "read", method_read, 1);
  rb_define_singleton_method(Stata_module, "write", method_write, 2);
}
