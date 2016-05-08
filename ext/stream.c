#include "extlzma.h"

VALUE extlzma_cStream;
static VALUE cEncoder;
static VALUE cDecoder;
static VALUE cAutoDecoder;
static VALUE cRawEncoder;
static VALUE cRawDecoder;

enum {
    WORK_BUFFER_SIZE = 256 * 1024, // 256 KiB
    BUFFER_BLOCK_SIZE = WORK_BUFFER_SIZE,
    UPDATE_TRY_MAX = 2,
};

static inline void
filter_copy(lzma_filter *dest, VALUE filter)
{
    memcpy(dest, extlzma_getfilter(filter), sizeof(*dest));
}

static void
stream_clear(lzma_stream *stream)
{
    static const lzma_stream init = LZMA_STREAM_INIT;
    memcpy(stream, &init, sizeof(init));
}

static inline lzma_stream *
getstream(VALUE lzma)
{
    return getref(lzma);
}

static inline void
stream_cleanup(void *pp)
{
    if (pp) {
        lzma_stream *p = (lzma_stream *)pp;
        lzma_end(p);
        free(p);
    }
}

static void
stream_mark(void *p)
{
}

static VALUE
stream_alloc(VALUE klass)
{
    lzma_stream *p;
    VALUE obj = Data_Make_Struct(klass, lzma_stream, stream_mark, stream_cleanup, p);
    stream_clear(p);
    return obj;
}

static VALUE
aux_str_reserve(VALUE str, size_t size)
{
    size_t capa = rb_str_capacity(str);
    if (capa < size) {
        rb_str_modify_expand(str, size - RSTRING_LEN(str));
    } else {
        rb_str_modify(str);
    }

    return str;
}

static inline void *
aux_lzma_code_nogvl(va_list *p)
{
    lzma_stream *stream = va_arg(*p, lzma_stream *);
    lzma_action sync = va_arg(*p, lzma_action);
    return (void *)lzma_code(stream, sync);
}

static inline lzma_ret
aux_lzma_code(lzma_stream *stream, lzma_action sync)
{
    return (lzma_ret)aux_thread_call_without_gvl(aux_lzma_code_nogvl, stream, sync);
}

/*
 * call-seq:
 *  code(src, dest, maxdest, action) -> status
 *
 * +lzma_code+ を用いて、圧縮/伸長処理を行います。
 *
 * [RETURN]
 *      +lzma_code+ が返す整数値をそのまま返します。
 *
 * [src]
 *      処理前のバイナリデータが格納された文字列オブジェクトを与えます。
 *
 *      このオブジェクトは変更されます。
 *
 *      処理された部分が取り除かれます (処理されなかった部分が残ります)。
 *
 * [dest]
 *      処理後のバイナリデータを格納する文字列オブジェクトを与えます。
 *
 * [maxdest]
 *      dest の最大処理バイト数を与えます。
 *
 * [action]
 *      +lzma_code+ の +action+ 引数に渡される整数値です。
 */
static VALUE
stream_code(VALUE stream, VALUE src, VALUE dest, VALUE maxdest, VALUE action)
{
    lzma_stream *p = getstream(stream);

    if (NIL_P(src)) {
        p->next_in = NULL;
        p->avail_in = 0;
    } else {
        rb_obj_infect(stream, src);
        rb_check_type(src, RUBY_T_STRING);
        rb_str_modify(src);
        p->next_in = (uint8_t *)RSTRING_PTR(src);
        p->avail_in = RSTRING_LEN(src);
    }

    size_t maxdestn = NUM2SIZET(maxdest);
    rb_check_type(dest, RUBY_T_STRING);
    aux_str_reserve(dest, maxdestn);
    rb_obj_infect(dest, stream);
    p->next_out = (uint8_t *)RSTRING_PTR(dest);
    p->avail_out = maxdestn;

    lzma_action act = NUM2INT(action);

    lzma_ret s = aux_lzma_code(p, act);

    if (p->next_in) {
        size_t srcrest = p->avail_in;
        memmove(RSTRING_PTR(src), p->next_in, srcrest);
        rb_str_set_len(src, srcrest);
    }

    rb_str_set_len(dest, maxdestn - p->avail_out);

    return UINT2NUM(s);
}

// filter は LZMA::Filter クラスのインスタンスを与えることができる
static void
filter_setup(lzma_filter filterpack[LZMA_FILTERS_MAX + 1], VALUE filter[], VALUE *filterend, VALUE encoder)
{
    if ((filterend - filter) > LZMA_FILTERS_MAX) {
        rb_raise(extlzma_eFilterTooLong,
                "filter chain too long (max %d, but given %d)",
                LZMA_FILTERS_MAX, (int)(filterend - filter));
    }
    for (; filter < filterend; filter ++, filterpack ++) {
        VALUE f = *filter;
        if (!rb_obj_is_kind_of(f, extlzma_cFilter)) {
            rb_raise(rb_eTypeError,
                    "not a filter - #<%s:%p>",
                    rb_obj_classname(f), (void *)f);
        }
        filter_copy(filterpack, f);
        OBJ_INFECT(encoder, f);
    }
    filterpack->id = LZMA_VLI_UNKNOWN;
    filterpack->options = NULL;
}

#define RETRY_NOMEM(TIMES, STMT)                                             \
    ({                                                                       \
        lzma_ret RETRY_NOMEM_status = 0;                                     \
        int RETRY_NOMEM_i;                                                   \
        for (RETRY_NOMEM_i = (TIMES); RETRY_NOMEM_i > 0; RETRY_NOMEM_i --) { \
            RETRY_NOMEM_status = ({STMT;});                                  \
            if (RETRY_NOMEM_status == LZMA_MEM_ERROR && RETRY_NOMEM_i > 1) { \
                rb_gc();                                                     \
                continue;                                                    \
            }                                                                \
            break;                                                           \
        }                                                                    \
        RETRY_NOMEM_status;                                                  \
    })                                                                       \

static int
conv_checkmethod(VALUE check)
{
    check = rb_hash_lookup2(check, ID2SYM(extlzma_id_check), Qundef);
    switch (check) {
    case Qnil:
        return LZMA_CHECK_NONE;
    case Qundef:
        return LZMA_CHECK_CRC64;
    default:
        if (check == ID2SYM(extlzma_id_none)) {
            return LZMA_CHECK_NONE;
        } else if (check == ID2SYM(extlzma_id_crc32)) {
            return LZMA_CHECK_CRC32;
        } else if (check == ID2SYM(extlzma_id_crc64)) {
            return LZMA_CHECK_CRC64;
        } else if (check == ID2SYM(extlzma_id_sha256)) {
            return LZMA_CHECK_SHA256;
        } else {
            return NUM2UINT(check);
        }
    }
}

static inline void
ext_encoder_init_scanargs(VALUE encoder, int argc, VALUE argv[], lzma_filter filterpack[LZMA_FILTERS_MAX + 1], uint32_t *check)
{
    if (check) {
        VALUE tmp;
        rb_scan_args(argc, argv, "13:", NULL, NULL, NULL, NULL, &tmp);
        if (NIL_P(tmp)) {
            *check = LZMA_CHECK_CRC64;
        } else {
            *check = conv_checkmethod(tmp);
            argc --;
        }
    } else {
        rb_scan_args(argc, argv, "13", NULL, NULL, NULL, NULL);
    }
    memset(filterpack, 0, sizeof(lzma_filter[LZMA_FILTERS_MAX + 1]));
    filter_setup(filterpack, argv, argv + argc, encoder);
}

/*
 * call-seq:
 *  initialize(filter1, check: CHECK_CRC64) -> encoder
 *  initialize(filter1, filter2, check: CHECK_CRC64) -> encoder
 *  initialize(filter1, filter2, filter3, check: CHECK_CRC64) -> encoder
 *  initialize(filter1, filter2, filter3, filter4, check: CHECK_CRC64) -> encoder
 *
 * 圧縮器を生成します。圧縮されたデータストリームは xz ファイルフォーマットです。
 *
 * [RETURN]
 *  生成された圧縮器
 *
 * [filter1, filter2, filter3, filter4]
 *  LZMA::Filter インスタンス。最低一つを必要とします。
 *
 * [check]
 *  チェックメソッド。
 *
 *  CHECK_NONE CHECK_CRC32 CHECK_CRC64 CHECK_SHA256 のいずれかの定数を与えます。
 *
 * [EXCEPTIONS]
 *      (NO DOCUMENTS)
 */
static VALUE
encoder_init(int argc, VALUE argv[], VALUE stream)
{
    lzma_stream *p = getstream(stream);

    uint32_t check;
    lzma_filter filterpack[LZMA_FILTERS_MAX + 1];
    ext_encoder_init_scanargs(stream, argc, argv, filterpack, &check);

    AUX_LZMA_TEST(RETRY_NOMEM(2, lzma_stream_encoder(p, filterpack, check)));

    return stream;
}

static inline void
ext_decoder_init_scanargs(int argc, VALUE argv[], uint64_t *memlimit, uint32_t *flags)
{
    switch (argc) {
    case 0:
        *memlimit = UINT64_MAX;
        *flags = 0;
        return;
    case 1:
        *memlimit = NIL_P(argv[0]) ? UINT64_MAX : NUM2SIZET(argv[0]);
        *flags = 0;
        return;
    case 2:
        *memlimit = NIL_P(argv[0]) ? UINT64_MAX : NUM2SIZET(argv[0]);
        *flags = NIL_P(argv[1]) ? 0 : (uint32_t)NUM2UINT(argv[1]);
        return;
    }

    rb_error_arity(argc, 0, 2);
}

/*
 * call-seq:
 *  initialize(memlimit = nil, flags = 0)
 *
 * [RETURN]
 *      伸張器を返します。
 *
 * [memlimit]
 *      作業メモリ量の最大値を指定します。単位はバイトです。
 *
 * [flags]
 *      伸張器の挙動を変更するための整数値を指定します。定数として次のものが利用できます。
 *
 *      - LZMA::TELL_NO_CHECK
 *      - LZMA::TELL_UNSUPPORTED_CHECK
 *      - LZMA::TELL_ANY_CHECK
 *      - LZMA::CONCATENATED
 *
 *      これらの意味は xz ユーティリティに含まれる liblzma/api/lzma/container.h に記述されています。
 */
static VALUE
autodecoder_init(int argc, VALUE argv[], VALUE stream)
{
    lzma_stream *p = getstream(stream);
    uint64_t memlimit;
    uint32_t flags;
    ext_decoder_init_scanargs(argc, argv, &memlimit, &flags);

    AUX_LZMA_TEST(RETRY_NOMEM(2, lzma_auto_decoder(p, memlimit, flags)));

    return stream;
}

/*
 * call-seq:
 *  initialize(memlimit = nil, flags = 0)
 *
 * xz ストリームの伸張器を返します。
 *
 * 引数については AutoDecoder#initialize と同じです。
 */
static VALUE
decoder_init(int argc, VALUE argv[], VALUE stream)
{
    lzma_stream *p = getstream(stream);

    uint64_t memlimit;
    uint32_t flags;
    ext_decoder_init_scanargs(argc, argv, &memlimit, &flags);

    AUX_LZMA_TEST(RETRY_NOMEM(2, lzma_stream_decoder(p, memlimit, flags)));

    return stream;
}

/*
 * call-seq:
 *  initialize(filter1) -> encoder
 *  initialize(filter1, filter2) -> encoder
 *  initialize(filter1, filter2, filter3) -> encoder
 *  initialize(filter1, filter2, filter3, filter4) -> encoder
 *
 * 生の (xzヘッダなどの付かない) LZMA1/LZMA2ストリームを構成する圧縮器を生成します。
 *
 * [RETURN]
 *      圧縮器を返します。
 *
 * [filter1, filter2, filter3, filter4]
 *      Filter インスタンスを与えます。
 *
 *      Filter インスタンスは、例えば LZMA2 フィルタを生成する場合 Filter.lzma2 を利用します。
 */
static VALUE
rawencoder_init(int argc, VALUE argv[], VALUE stream)
{
    lzma_stream *p = getstream(stream);

    lzma_filter filterpack[LZMA_FILTERS_MAX + 1];
    ext_encoder_init_scanargs(stream, argc, argv, filterpack, NULL);

    AUX_LZMA_TEST(RETRY_NOMEM(2, lzma_raw_encoder(p, filterpack)));

    return stream;
}

/*
 * call-seq:
 *  initialize(filter1) -> decoder
 *  initialize(filter1, filter2) -> decoder
 *  initialize(filter1, filter2, filter3) -> decoder
 *  initialize(filter1, filter2, filter3, filter4) -> decoder
 */
static VALUE
rawdecoder_init(int argc, VALUE argv[], VALUE stream)
{
    lzma_stream *p = getstream(stream);

    lzma_filter filterpack[LZMA_FILTERS_MAX + 1];
    ext_encoder_init_scanargs(stream, argc, argv, filterpack, NULL);

    AUX_LZMA_TEST(RETRY_NOMEM(2, lzma_raw_decoder(p, filterpack)));

    return stream;
}

void
extlzma_init_Stream(void)
{
    extlzma_cStream = rb_define_class_under(extlzma_mLZMA, "Stream", rb_cObject);
    rb_undef_alloc_func(extlzma_cStream);
    rb_define_method(extlzma_cStream, "code", stream_code, 4);

    cEncoder = rb_define_class_under(extlzma_cStream, "Encoder", extlzma_cStream);
    rb_define_alloc_func(cEncoder, stream_alloc);
    rb_define_method(cEncoder, "initialize", RUBY_METHOD_FUNC(encoder_init), -1);
    rb_define_alias(cEncoder, "encode", "code");
    rb_define_alias(cEncoder, "compress", "code");

    cDecoder = rb_define_class_under(extlzma_cStream, "Decoder", extlzma_cStream);
    rb_define_alloc_func(cDecoder, stream_alloc);
    rb_define_method(cDecoder, "initialize", RUBY_METHOD_FUNC(decoder_init), -1);
    rb_define_alias(cDecoder, "decode", "code");
    rb_define_alias(cDecoder, "decompress", "code");
    rb_define_alias(cDecoder, "uncompress", "code");

    cAutoDecoder = rb_define_class_under(extlzma_cStream, "AutoDecoder", extlzma_cStream);
    rb_define_alloc_func(cAutoDecoder, stream_alloc);
    rb_define_method(cAutoDecoder, "initialize", RUBY_METHOD_FUNC(autodecoder_init), -1);
    rb_define_alias(cDecoder, "decode", "code");
    rb_define_alias(cDecoder, "decompress", "code");
    rb_define_alias(cDecoder, "uncompress", "code");

    cRawEncoder = rb_define_class_under(extlzma_cStream, "RawEncoder", extlzma_cStream);
    rb_define_alloc_func(cRawEncoder, stream_alloc);
    rb_define_method(cRawEncoder, "initialize", RUBY_METHOD_FUNC(rawencoder_init), -1);
    rb_define_alias(cEncoder, "encode", "code");
    rb_define_alias(cEncoder, "compress", "code");

    cRawDecoder = rb_define_class_under(extlzma_cStream, "RawDecoder", extlzma_cStream);
    rb_define_alloc_func(cRawDecoder, stream_alloc);
    rb_define_method(cRawDecoder, "initialize", RUBY_METHOD_FUNC(rawdecoder_init), -1);
    rb_define_alias(cDecoder, "decode", "code");
    rb_define_alias(cDecoder, "decompress", "code");
    rb_define_alias(cDecoder, "uncompress", "code");
}
