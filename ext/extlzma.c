/*
 * liblzma.c -
 * - Author:    dearblue <dearblue@users.osdn.me>
 * - Copyright: Copyright (c) 2010 dearblue
 * - License:   Distributed under the 2-clause BSD License
 */

#include "extlzma.h"

VALUE extlzma_mLZMA;

ID extlzma_id_dictsize;
ID extlzma_id_predict;
ID extlzma_id_lc;
ID extlzma_id_lp;
ID extlzma_id_pb;
ID extlzma_id_mode;
ID extlzma_id_nice;
ID extlzma_id_mf;
ID extlzma_id_depth;
ID extlzma_id_check;
ID extlzma_id_none;
ID extlzma_id_crc32;
ID extlzma_id_crc64;
ID extlzma_id_sha256;

static VALUE
libver_major(VALUE obj)
{
    return INT2FIX(LZMA_VERSION_MAJOR);
}

static VALUE
libver_minor(VALUE obj)
{
    return INT2FIX(LZMA_VERSION_MINOR);
}

static VALUE
libver_patch(VALUE obj)
{
    return INT2FIX(LZMA_VERSION_PATCH);
}

static VALUE
libver_to_s(VALUE obj)
{
    return rb_str_new_cstr(LZMA_VERSION_STRING);
}


static void
extlzma_init_LIBVER(void)
{
    VALUE vers[] = {
        INT2FIX(LZMA_VERSION_MAJOR),
        INT2FIX(LZMA_VERSION_MINOR),
        INT2FIX(LZMA_VERSION_PATCH)
    };

    VALUE libver = rb_ary_new4(ELEMENTOF(vers), vers);
    rb_define_singleton_method(libver, "major", RUBY_METHOD_FUNC(libver_major), 0);
    rb_define_singleton_method(libver, "minor", RUBY_METHOD_FUNC(libver_minor), 0);
    rb_define_singleton_method(libver, "patch", RUBY_METHOD_FUNC(libver_patch), 0);
    rb_define_singleton_method(libver, "to_s", RUBY_METHOD_FUNC(libver_to_s), 0);
    rb_define_const(extlzma_mLZMA, "LIBRARY_VERSION", rb_ary_freeze(libver));
}

void
Init_extlzma(void)
{
    extlzma_id_dictsize = rb_intern("dictsize");
    extlzma_id_predict  = rb_intern("predict");
    extlzma_id_lc       = rb_intern("lc");
    extlzma_id_lp       = rb_intern("lp");
    extlzma_id_pb       = rb_intern("pb");
    extlzma_id_mode     = rb_intern("mode");
    extlzma_id_nice     = rb_intern("nice");
    extlzma_id_mf       = rb_intern("mf");
    extlzma_id_depth    = rb_intern("depth");
    extlzma_id_check    = rb_intern("check");
    extlzma_id_none     = rb_intern("none");
    extlzma_id_crc32    = rb_intern("crc32");
    extlzma_id_crc64    = rb_intern("crc64");
    extlzma_id_sha256   = rb_intern("sha256");

    extlzma_mLZMA = rb_define_module("LZMA");
    rb_define_const(extlzma_mLZMA, "LZMA", extlzma_mLZMA);

    extlzma_init_Utils();
    extlzma_init_Constants();
    extlzma_init_Exceptions();
    extlzma_init_Filter();
    extlzma_init_Stream();
    extlzma_init_Index();
    extlzma_init_LIBVER();
}
