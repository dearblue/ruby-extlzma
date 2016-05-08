
#include "extlzma.h"

static VALUE cIndex;
static VALUE cIEncoder;
static VALUE cIDecoder;


static void
ext_index_free(lzma_index *index)
{
    if (index) {
        lzma_index_end(index, NULL);
    }
}

static VALUE
ext_index_alloc(VALUE klass)
{
    lzma_index *index = lzma_index_init(NULL);
    if (!index) {
        rb_raise(rb_eNoMemError, "failed allocation for lzma index structure");
    }
    return Data_Wrap_Struct(klass, NULL, ext_index_free, index);
}

static lzma_index *
ext_index_refp(VALUE index)
{
    return getrefp(index);
}

static lzma_index *
ext_index_ref(VALUE index)
{
    return getref(index);
}


static VALUE
iencoder_init(int argc, VALUE argv[], VALUE idx)
{
    //RDATA_PTR(idx);

    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE
idecoder_init(int argc, VALUE argv[], VALUE idx)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE
ext_index_memused(VALUE index)
{
    return ULL2NUM(lzma_index_memused(ext_index_ref(index)));
}

static VALUE
ext_index_s_memusage(VALUE index, VALUE streams, VALUE blocks)
{
    return ULL2NUM(lzma_index_memusage(NUM2ULL(streams), NUM2ULL(blocks)));
}

void
extlzma_init_Index(void)
{
    cIndex = rb_define_class_under(extlzma_mLZMA, "Index", rb_cObject);
    rb_undef_alloc_func(cIndex);
    rb_define_singleton_method(cIndex, "memusage", RUBY_METHOD_FUNC(ext_index_s_memusage), 2);
    rb_define_method(cIndex, "memused", RUBY_METHOD_FUNC(ext_index_memused), 0);

    cIEncoder = rb_define_class_under(cIndex, "Encoder", cIndex);
    rb_define_alloc_func(cIEncoder, ext_index_alloc);
    rb_define_method(cIEncoder, "initialize", RUBY_METHOD_FUNC(iencoder_init), -1);

    cIDecoder = rb_define_class_under(cIndex, "Decoder", cIndex);
    rb_define_alloc_func(cIDecoder, ext_index_alloc);
    rb_define_method(cIDecoder, "initialize", RUBY_METHOD_FUNC(idecoder_init), -1);
}
