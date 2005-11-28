#include "ferret.h"


/****************************************************************************
 *
 * Term Methods
 *
 ****************************************************************************/

void
frt_term_free(void *p)
{
	Term *term = (Term *)p;
	free(term->text);
	free(p);
}

void
frt_term_mark(void *p)
{
	Term *term = (Term *)p;
  rb_gc_mark(term->field);
}

static VALUE
frt_term_alloc(VALUE klass)
{
	Term *term = ALLOC(Term);
  MEMZERO(term, Term, 1);
  term->field = Qnil;
	return Data_Wrap_Struct(klass, frt_term_mark, frt_term_free, term);
}

#define GET_TERM Term *term; Data_Get_Struct(self, Term, term)
VALUE 
frt_term_set(VALUE self, VALUE rfield, VALUE rtext)
{
  GET_TERM;

  int tlen = RSTRING(rtext)->len;
  term->field = rfield;
	REALLOC_N(term->text, char, tlen + 1);
	MEMCPY(term->text, RSTRING(rtext)->ptr, char, tlen);
	term->tlen = tlen;
	
	return Qnil;
}

static VALUE 
frt_term_init(VALUE self, VALUE rfield, VALUE rtext)
{
	frt_term_set(self, rfield, rtext);
	return self;
}

static VALUE 
frt_term_get_text(VALUE self)
{
  GET_TERM;
	return rb_str_new(term->text, term->tlen);
}

static VALUE 
frt_term_set_text(VALUE self, VALUE rtext)
{
  GET_TERM;
	int tlen = RSTRING(rtext)->len;
 	char *text = RSTRING(rtext)->ptr;

	REALLOC_N(term->text, char, tlen + 1);
	
	MEMCPY(term->text, text, char, tlen);
	term->tlen = tlen;
	
	return Qnil;
}

static VALUE 
frt_term_get_field(VALUE self)
{
  GET_TERM;
	return term->field;
}

static VALUE 
frt_term_set_field(VALUE self, VALUE rfield)
{
  GET_TERM;
  term->field = rfield;
	return Qnil;
}

VALUE 
frt_term_to_s(VALUE self)
{
  GET_TERM;
	int tlen, flen;
	tlen = term->tlen;
	flen = RSTRING(term->field)->len;
	char res[flen + tlen + 1];
	char delim[] = ":";
	
	MEMCPY(res, StringValuePtr(term->field), char, flen);
	MEMCPY(res + flen, delim, char, 1);
	MEMCPY(res + flen + 1, term->text, char, tlen);
	return rb_str_new(res, tlen + flen + 1 );
}

int
frt_term_compare_to_int(VALUE self, VALUE rother)
{
  GET_TERM;
	Term *other; Data_Get_Struct(rother, Term, other);
	int comp, size, my_len, o_len;
		
	my_len = RSTRING(term->field)->len;
	o_len = RSTRING(other->field)->len;
	size = my_len >= o_len ? o_len : my_len;
	comp = memcmp(RSTRING(term->field)->ptr, RSTRING(other->field)->ptr, size);
	if(comp == 0){
		if(my_len == o_len) {
			my_len = term->tlen;
			o_len = other->tlen;
			size = my_len >= o_len ? o_len : my_len;
			comp = memcmp(term->text, other->text, size);
			if(comp == 0 && my_len != o_len)
				comp = my_len > o_len ? 1 : -1;
		} else {
			comp = my_len > o_len ? 1 : -1;
    }
	}
	return comp;
}

VALUE
frt_term_lt(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) < 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_gt(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) > 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_le(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) <= 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_ge(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) >= 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_eq(VALUE self, VALUE rother)
{
  if (rother == Qnil)
    return Qfalse;
  return frt_term_compare_to_int(self, rother) == 0 ? Qtrue : Qfalse;
}


static VALUE
frt_term_compare_to(VALUE self, VALUE other)
{
	return INT2FIX(frt_term_compare_to_int(self, other));
}

static VALUE
frt_term_hash(VALUE self)
{
  GET_TERM;
  return INT2FIX(frt_hash(term->text, term->tlen) +
      frt_hash(RSTRING(term->field)->ptr, RSTRING(term->field)->len));
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_term(void)
{
	/* Term */
	cTerm = rb_define_class_under(mIndex, "Term", rb_cObject);
	rb_define_alloc_func(cTerm, frt_term_alloc);
	rb_include_module(cTerm, rb_mComparable);

	rb_define_method(cTerm, "initialize", frt_term_init, 2);
	rb_define_method(cTerm, "set!", frt_term_set, 2);
	rb_define_method(cTerm, "to_s", frt_term_to_s, 0);
	rb_define_method(cTerm, "<=>", frt_term_compare_to, 1);
	rb_define_method(cTerm, "<", frt_term_lt, 1);
	rb_define_method(cTerm, ">", frt_term_gt, 1);
	rb_define_method(cTerm, "<=", frt_term_le, 1);
	rb_define_method(cTerm, ">=", frt_term_ge, 1);
	rb_define_method(cTerm, "eql?", frt_term_eq, 1);
	rb_define_method(cTerm, "==", frt_term_eq, 1);
	rb_define_method(cTerm, "text", frt_term_get_text, 0);
	rb_define_method(cTerm, "text=", frt_term_set_text, 1);
	rb_define_method(cTerm, "field", frt_term_get_field, 0);
	rb_define_method(cTerm, "field=", frt_term_set_field, 1);
	rb_define_method(cTerm, "hash", frt_term_hash, 0);
}
