#include "extlzma.h"

static inline VALUE
crc_calc(void *(*update)(va_list *vp), int argc, VALUE argv[])
{
    VALUE src, crc;
    rb_scan_args(argc, argv, "11", &src, &crc);
    rb_check_type(src, RUBY_T_STRING);
    return (VALUE)aux_thread_call_without_gvl(update,
            (const uint8_t *)RSTRING_PTR(src),
            RSTRING_LEN(src),
            NIL_P(crc) ? INT2FIX(0) : crc);
}


static VALUE
crc32_update(va_list *vp)
{
    const uint8_t *ptr = va_arg(*vp, const uint8_t *);
    size_t size = va_arg(*vp, size_t);
    VALUE crc = va_arg(*vp, VALUE);
    return UINT2NUM(lzma_crc32(ptr, size, NUM2UINT(crc)));
}

/*
 * call-seq:
 *  LZMA::Utils.crc32(string, crc = 0)
 *
 * liblzmaに含まれるlzma_crc32を呼び出します。
 */
static VALUE
utils_crc32(int argc, VALUE argv[], VALUE self)
{
    return crc_calc((void *(*)(va_list *))crc32_update, argc, argv);
}


static VALUE
crc64_update(va_list *vp)
{
    const uint8_t *ptr = va_arg(*vp, const uint8_t *);
    size_t size = va_arg(*vp, size_t);
    VALUE crc = va_arg(*vp, VALUE);
    return ULL2NUM(lzma_crc64(ptr, size, NUM2ULL(crc)));
}

/*
 * call-seq:
 *  LZMA::Utils.crc64(string, crc = 0)
 *
 * liblzmaに含まれるlzma_crc64を呼び出します。
 */
static VALUE
utils_crc64(int argc, VALUE argv[], VALUE self)
{
    return crc_calc((void *(*)(va_list *))crc64_update, argc, argv);
}

/*
 * call-seq:
 *  lookup_error(lzma_ret) -> exception class
 */
static VALUE
utils_lookup_error(VALUE mod, VALUE status)
{
    return extlzma_lookup_error(NUM2INT(status));
}

/*
 * call-seq:
 *  stream_buffer_bound(uncompressed_size) -> worst encoded size
 */
static VALUE
utils_stream_buffer_bound(VALUE mod, VALUE size)
{
    return SIZET2NUM(lzma_stream_buffer_bound(NUM2SIZET(size)));
}

/*
 * call-seq:
 *  block_buffer_bound(uncompressed_size) -> worst encoded size
 */
static VALUE
utils_block_buffer_bound(VALUE mod, VALUE size)
{
    return SIZET2NUM(lzma_block_buffer_bound(NUM2SIZET(size)));
}


static VALUE mUtils;

void
extlzma_init_Utils(void)
{
    mUtils = rb_define_module_under(extlzma_mLZMA, "Utils");

    rb_extend_object(extlzma_mLZMA, mUtils);
    rb_extend_object(mUtils, mUtils); // 自分に対してもモジュールメソッドを利用できるようにする

    rb_define_method(mUtils, "crc32", RUBY_METHOD_FUNC(utils_crc32), -1);
    rb_define_method(mUtils, "crc64", RUBY_METHOD_FUNC(utils_crc64), -1);
    rb_define_method(mUtils, "lookup_error", RUBY_METHOD_FUNC(utils_lookup_error), 1);
    rb_define_method(mUtils, "stream_buffer_bound", RUBY_METHOD_FUNC(utils_stream_buffer_bound), 1);
    rb_define_method(mUtils, "block_buffer_bound", RUBY_METHOD_FUNC(utils_block_buffer_bound), 1);
}
