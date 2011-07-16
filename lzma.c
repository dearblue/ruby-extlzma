/*
 * lzma.c -
 * Author::    dearblue
 * Copyright:: Copyright (c) 2010 dearblue
 * License::   Distributes under the same 2-causes BSD License
 */


#include <ruby.h>
#include <ruby/intern.h>
#include <lzma.h>


#define RUBY_BEGIN_BLOCKING_REGION  \
    ({                              \
        VALUE UPDATE$$(void)        \
        {                           \

#define RUBY_END_BLOCKING_REGION                                                         \
        }                                                                                \
        rb_thread_blocking_region((VALUE (*)(void *))UPDATE$$, NULL, RUBY_UBF_IO, NULL); \
    })                                                                                   \



static VALUE mLZMA;


static VALUE symDICTSIZE;
static VALUE symPRESETDICT;
static VALUE symLC;
static VALUE symLP;
static VALUE symPB;
static VALUE symMODE;
static VALUE symNICE;
static VALUE symMF;
static VALUE symDEPTH;
static VALUE symCHECK;


static inline int lzma_isfailed(lzma_ret status)
{
    return status != 0;
}


// SECTION: LZMA::Constants

static VALUE mConstants;

static void setup_constants(void)
{
    mConstants = rb_define_module_under(mLZMA, "Constants");
    rb_include_module(mLZMA, mConstants);

#define DEFINE_CONSTANT(NAME, VALUE)                       \
    {                                                      \
        rb_define_const(mConstants, #NAME, VALUE);         \
        rb_define_const(mConstants, "LZMA_" #NAME, VALUE); \
    }                                                      \

    DEFINE_CONSTANT(NICE_DEFAULT,       SIZET2NUM(64));
    DEFINE_CONSTANT(DEPTH_DEFAULT,      SIZET2NUM(0));

    DEFINE_CONSTANT(PRESET_DEFAULT,     INT2NUM(LZMA_PRESET_DEFAULT));
    DEFINE_CONSTANT(PRESET_LEVEL_MASK,  INT2NUM(LZMA_PRESET_LEVEL_MASK));
    DEFINE_CONSTANT(PRESET_EXTREME,     INT2NUM(LZMA_PRESET_EXTREME));
    DEFINE_CONSTANT(DICT_SIZE_MIN,      INT2NUM(LZMA_DICT_SIZE_MIN));
    DEFINE_CONSTANT(DICT_SIZE_DEFAULT,  INT2NUM(LZMA_DICT_SIZE_DEFAULT));
    DEFINE_CONSTANT(LCLP_MIN,           INT2NUM(LZMA_LCLP_MIN));
    DEFINE_CONSTANT(LCLP_MAX,           INT2NUM(LZMA_LCLP_MAX));
    DEFINE_CONSTANT(LC_DEFAULT,         INT2NUM(LZMA_LC_DEFAULT));
    DEFINE_CONSTANT(LP_DEFAULT,         INT2NUM(LZMA_LP_DEFAULT));
    DEFINE_CONSTANT(PB_MIN,             INT2NUM(LZMA_PB_MIN));
    DEFINE_CONSTANT(PB_MAX,             INT2NUM(LZMA_PB_MAX));
    DEFINE_CONSTANT(PB_DEFAULT,         INT2NUM(LZMA_PB_DEFAULT));
    DEFINE_CONSTANT(MODE_FAST,          INT2NUM(LZMA_MODE_FAST));
    DEFINE_CONSTANT(MODE_NORMAL,        INT2NUM(LZMA_MODE_NORMAL));
    DEFINE_CONSTANT(MF_HC3,             INT2NUM(LZMA_MF_HC3));
    DEFINE_CONSTANT(MF_HC4,             INT2NUM(LZMA_MF_HC4));
    DEFINE_CONSTANT(MF_BT2,             INT2NUM(LZMA_MF_BT2));
    DEFINE_CONSTANT(MF_BT3,             INT2NUM(LZMA_MF_BT3));
    DEFINE_CONSTANT(MF_BT4,             INT2NUM(LZMA_MF_BT4));
                                       
    DEFINE_CONSTANT(CHECK_NONE,         INT2NUM(LZMA_CHECK_NONE));
    DEFINE_CONSTANT(CHECK_CRC32,        INT2NUM(LZMA_CHECK_CRC32));
    DEFINE_CONSTANT(CHECK_CRC64,        INT2NUM(LZMA_CHECK_CRC64));
    DEFINE_CONSTANT(CHECK_SHA256,       INT2NUM(LZMA_CHECK_SHA256));
                                       
    DEFINE_CONSTANT(RUN,                INT2NUM(LZMA_RUN));
    DEFINE_CONSTANT(FULL_FLUSH,         INT2NUM(LZMA_FULL_FLUSH));
    DEFINE_CONSTANT(SYNC_FLUSH,         INT2NUM(LZMA_SYNC_FLUSH));
    DEFINE_CONSTANT(FINISH,             INT2NUM(LZMA_FINISH));

#undef DEFINE_CONSTANT
}


// SECTION: LZMA::Exceptions

static VALUE mExceptions;

static VALUE eBasicException;
static VALUE eStreamEnd;
static VALUE eNoCheck;
static VALUE eUnsupportedCheck;
static VALUE eGetCheck;
static VALUE eMemError;
static VALUE eMemlimitError;
static VALUE eFormatError;
static VALUE eOptionsError;
static VALUE eDataError;
static VALUE eBufError;
static VALUE eProgError;
static VALUE eFilterTooLong;
static VALUE eBadPreset;

static inline VALUE lookup_exception(lzma_ret status)
{
    switch (status) {
    case LZMA_OK:                return Qnil;
    case LZMA_STREAM_END:        return eStreamEnd;
    case LZMA_NO_CHECK:          return eNoCheck;
    case LZMA_UNSUPPORTED_CHECK: return eUnsupportedCheck;
    case LZMA_GET_CHECK:         return eGetCheck;
    case LZMA_MEM_ERROR:         return eMemError;
    case LZMA_MEMLIMIT_ERROR:    return eMemlimitError;
    case LZMA_FORMAT_ERROR:      return eFormatError;
    case LZMA_OPTIONS_ERROR:     return eOptionsError;
    case LZMA_DATA_ERROR:        return eDataError;
    case LZMA_BUF_ERROR:         return eBufError;
    case LZMA_PROG_ERROR:        return eProgError;
    default:                     return rb_eRuntimeError;
    }
}

static void setup_exceptions(void)
{
    mExceptions = rb_define_module_under(mLZMA, "Exceptions");
    rb_include_module(mLZMA, mExceptions);

    eBasicException = rb_define_class_under(mExceptions, "BasicException", rb_eStandardError);
    rb_define_class_under(mExceptions, "FilterTooLong", eBasicException);
    rb_define_class_under(mExceptions, "BadPreset", eBasicException);

#define DEFINE_EXCEPTION(CLASS, STATUS)                                           \
    {                                                                             \
        e ## CLASS = rb_define_class_under(mExceptions, #CLASS, eBasicException); \
        rb_define_const(e ## CLASS, "STATUS", SIZET2NUM(STATUS));                 \
    }                                                                             \

    DEFINE_EXCEPTION(StreamEnd,        LZMA_STREAM_END);
    DEFINE_EXCEPTION(NoCheck,          LZMA_NO_CHECK);
    DEFINE_EXCEPTION(UnsupportedCheck, LZMA_UNSUPPORTED_CHECK);
    DEFINE_EXCEPTION(GetCheck,         LZMA_GET_CHECK);
    DEFINE_EXCEPTION(MemError,         LZMA_MEM_ERROR);
    DEFINE_EXCEPTION(MemlimitError,    LZMA_MEMLIMIT_ERROR);
    DEFINE_EXCEPTION(FormatError,      LZMA_FORMAT_ERROR);
    DEFINE_EXCEPTION(OptionsError,     LZMA_OPTIONS_ERROR);
    DEFINE_EXCEPTION(DataError,        LZMA_DATA_ERROR);
    DEFINE_EXCEPTION(BufError,         LZMA_BUF_ERROR);
    DEFINE_EXCEPTION(ProgError,        LZMA_PROG_ERROR);
#undef DEFINE_EXCEPTION
}


// SECTION: LZMA::Filter

static VALUE cFilter;
static VALUE cBasicLZMA;
static VALUE cLZMA1;
static VALUE cLZMA2;
static VALUE cDelta;


static inline lzma_filter *getfilter(VALUE obj)
{
    lzma_filter *filter;
    Data_Get_Struct(obj, lzma_filter, filter);
    return filter;
}

static void *setup_lzma_preset(size_t preset)
{
    lzma_options_lzma *lzma = xcalloc(sizeof(lzma_options_lzma), 1);
    lzma_lzma_preset(lzma, preset);
    return (void *)(lzma);
}

static const char PRIVATE_DICTPRESET[] = "dict_preset";

static inline void lzma_set_dict_0(lzma_options_lzma *filter, VALUE dict, VALUE self)
{
    if (NIL_P(dict)) {
        filter->preset_dict = NULL;
        filter->preset_dict_size = 0;
    } else {
        if (TYPE(dict) != T_STRING) {
            rb_raise(rb_eTypeError, "%s", "dict is not a String or nil");
        }
        dict = rb_str_new_frozen(dict);
        filter->preset_dict = (uint8_t *)(RSTRING_PTR(dict));
        filter->preset_dict_size = RSTRING_LEN(dict);
    }
    rb_iv_set(self, PRIVATE_DICTPRESET, dict);
}

static VALUE lzma_set_dict(VALUE self, VALUE dict)
{
    lzma_set_dict_0((lzma_options_lzma *)getfilter(self)->options, dict, self);
    return self;
}

static VALUE lzma_get_dict(VALUE self)
{
    return rb_str_new_shared(rb_iv_get(self, PRIVATE_DICTPRESET));
}


#define DEFINE_ACCESSOR_ENTITY(NAME, MEMBER, DEFAULT)                              \
    static inline void lzma_set_ ## NAME ## _0(lzma_options_lzma *filter, VALUE n) \
    {                                                                              \
        if (NIL_P(n)) {                                                            \
            filter->MEMBER = DEFAULT;                                              \
        } else {                                                                   \
            filter->MEMBER = NUM2UINT(n);                                          \
        }                                                                          \
    }                                                                              \
                                                                                   \
    static VALUE lzma_set_ ## NAME(VALUE self, VALUE n)                            \
    {                                                                              \
        lzma_set_ ## NAME ## _0((lzma_options_lzma *)getfilter(self)->options, n); \
        return self;                                                               \
    }                                                                              \
                                                                                   \
    static VALUE lzma_get_ ## NAME(VALUE self)                                     \
    {                                                                              \
        return UINT2NUM(((lzma_options_lzma *)getfilter(self)->options)->MEMBER);  \
    }                                                                              \

DEFINE_ACCESSOR_ENTITY(dictsize,   dict_size,   LZMA_DICT_SIZE_DEFAULT)
DEFINE_ACCESSOR_ENTITY(lc,         lc,          LZMA_LC_DEFAULT)
DEFINE_ACCESSOR_ENTITY(lp,         lp,          LZMA_LP_DEFAULT)
DEFINE_ACCESSOR_ENTITY(pb,         pb,          LZMA_PB_DEFAULT)
DEFINE_ACCESSOR_ENTITY(mode,       mode,        LZMA_MODE_NORMAL)
DEFINE_ACCESSOR_ENTITY(nice,       nice_len,    64)
DEFINE_ACCESSOR_ENTITY(mf,         mf,          LZMA_MF_BT4)
DEFINE_ACCESSOR_ENTITY(depth,      depth,       0)

#undef DEFINE_ACCESSOR_ENTITY

static void *setup_lzma(VALUE obj, uint32_t preset,
                        VALUE dictsize, VALUE dictpreset, VALUE lc, VALUE lp, VALUE pb,
                        VALUE mode, VALUE nice, VALUE mf, VALUE depth)
{
    lzma_options_lzma lzma = { 0 };
    if (lzma_lzma_preset(&lzma, preset) != 0) {
        rb_raise(eBadPreset, "bad preset (%08x)", preset);
    }

    if (RTEST(dictpreset)) { lzma_set_dict_0(&lzma, dictpreset, obj); }

#define SETVAR(NAME)                                           \
    if (RTEST(NAME)) { lzma_set_ ## NAME ## _0(&lzma, NAME); } \

    SETVAR(dictsize);
    SETVAR(lc);
    SETVAR(lp);
    SETVAR(pb);
    SETVAR(mode);
    SETVAR(nice);
    SETVAR(mf);
    SETVAR(depth);

#undef SETVAR

    {
        lzma_options_lzma *p = ALLOC(lzma_options_lzma);
        memcpy(p, &lzma, sizeof(lzma));
        return (void *)(p);
    }
}

static void cleanup_filter(lzma_filter *filter)
{
    if (filter->options) { xfree(filter->options); }
    xfree(filter);
}


static void setup_filter_lzmaX_scan(int argc, VALUE argv[], VALUE *preset, VALUE *opts)
{
    switch (rb_scan_args(argc, argv, "02", preset, opts)) {
    case 0:
        *preset = *opts = Qnil;
        break;
    case 1:
        if (TYPE(*preset) == T_HASH) {
            *opts = *preset;
            *preset = Qnil;
        } else {
            *opts = Qnil;
        }
        break;
    case 2:
        if (TYPE(*opts) != T_HASH) { rb_raise(rb_eTypeError, "argument-2 is not a hash"); }
        break;
    default:
        rb_bug("%s:%d", __FILE__, __LINE__);
    }
}

static uint32_t getpreset(VALUE preset)
{
    if (NIL_P(preset)) {
        return LZMA_PRESET_DEFAULT;
    } else {
        return NUM2UINT(preset);
    }
}

/*
 * call-seq:
 *  LZMA::Filter.lzma1(....)
 *
 * LZMA::Filter::LZMA1.newを呼ぶための補助機構である
 *
 * LZMA::Filter::LZMA1クラスの .new や #initialize を書き換えた場合はそれに従う
 * (あくまで .new を呼ぶだけである)
 */
static VALUE lzma1_new(int argc, VALUE argv[], VALUE self)
{
    return rb_class_new_instance(argc, argv, cLZMA1);
}

/*
 * call-seq:
 *  LZMA::Filter.lzma2(...)
 *
 * LZMA::Filter::LZMA2.new を呼ぶための補助機構である
 *
 * LZMA::Filter::LZMA2クラスの .new や #initialize を書き換えた場合はそれに従う
 * (あくまで .new を呼ぶだけである)
 */
static VALUE lzma2_new(int argc, VALUE argv[], VALUE self)
{
    return rb_class_new_instance(argc, argv, cLZMA2);
}

/*
 * call-seq:
 *  LZMA::Filter.delta(....)
 *
 * LZMA::Filter::Delta.newを呼ぶための補助機構である
 *
 * LZMA::Filter::Deltaクラスの .new や #initialize を書き換えた場合はそれに従う
 * (あくまで .new を呼ぶだけである)
 */
static VALUE delta_new(int argc, VALUE argv[], VALUE self)
{
    return rb_class_new_instance(argc, argv, cDelta);
}



static inline VALUE filter_alloc(VALUE klass, lzma_vli id)
{
    lzma_filter *filter;
    VALUE obj = Data_Make_Struct(klass, lzma_filter, NULL, cleanup_filter, filter);
    memset(filter, 0, sizeof(*filter));
    filter->id = id;
    return obj;
}

static VALUE lzma1_alloc(VALUE klass)
{
    return filter_alloc(klass, LZMA_FILTER_LZMA1);
}

static VALUE lzma2_alloc(VALUE klass)
{
    return filter_alloc(klass, LZMA_FILTER_LZMA2);
}

static VALUE delta_alloc(VALUE klass)
{
    return filter_alloc(cDelta, LZMA_FILTER_DELTA);
}

/*
 * call-seq:
 *  LZMA::Filter::Delta.initialize(dist = LZMA::DELTA_DIST_MIN)
 *
 * 差分フィルタ設定オブジェクトを返す
 */
static VALUE delta_init(int argc, VALUE argv[], VALUE self)
{
    lzma_filter *filter = getfilter(self);
    lzma_options_delta *delta = ALLOC(lzma_options_delta);
    memset(delta, 0, sizeof(*delta));

    VALUE preset = Qnil;
    rb_scan_args(argc, argv, "01", &preset);
    delta->type = LZMA_DELTA_TYPE_BYTE;
    delta->dist = NIL_P(preset) ? LZMA_DELTA_DIST_MIN : NUM2UINT(preset);
    filter->options = delta;
    return self;
}

static inline VALUE lzmaX_init(int argc, VALUE argv[], VALUE self)
{
    VALUE preset = Qnil;
    VALUE opts = Qnil;
    setup_filter_lzmaX_scan(argc, argv, &preset, &opts);
    lzma_filter *filter = getfilter(self);
    if (NIL_P(opts)) {
        filter->options = setup_lzma_preset(getpreset(preset));
    } else {
        filter->options = setup_lzma(self, getpreset(preset),
                                     rb_hash_lookup(opts, symDICTSIZE),
                                     rb_hash_lookup(opts, symPRESETDICT),
                                     rb_hash_lookup(opts, symLC),
                                     rb_hash_lookup(opts, symLP),
                                     rb_hash_lookup(opts, symPB),
                                     rb_hash_lookup(opts, symMODE),
                                     rb_hash_lookup(opts, symNICE),
                                     rb_hash_lookup(opts, symMF),
                                     rb_hash_lookup(opts, symDEPTH));
    }
    return self;
}

/*
 * call-seq:
 *  LZMA::Filter::LZMA1.initialize(preset = LZMA::PRESET_DEFAULT, opts = { .... } )
 *  LZMA::Filter::LZMA1.initialize(opts)
 *  LZMA::Filter::LZMA2.initialize( .... )
 *
 * call-seq:
 *  LZMA::Filter::LZMA1.initialize(dictsize: LZMA::DICT_SIZE_DEFAULT,
 *                                 presetdict: nil,
 *                                 lc: LZMA::LC_DEFAULT,
 *                                 lp: LZMA::LP_DEFAULT,
 *                                 pb: LZMA::PB_DEFAULT,
 *                                 mode: LZMA::MODE_NORMAL,
 *                                 nice: LZMA::NICE_DEFAULT,
 *                                 mf: LZMA::MF_BT4, # match finder
 *                                 depth: LZMA::DEPTH_DEFAULT)
 *
 * フィルタ設定オブジェクトを返す。
 *
 * この段階で各値の確認を行うことはせず、*encoderに渡すときに初めて確認される
 */
static VALUE lzma1_init(int argc, VALUE argv[], VALUE self)
{
    return lzmaX_init(argc, argv, self);
}

/*
 * 引数の取り方はLZMA::Filter::LZMA1.initializeと同じである
 */
static VALUE lzma2_init(int argc, VALUE argv[], VALUE self)
{
    return lzmaX_init(argc, argv, self);
}

/**/
static void setup_filter(void)
{
    cFilter = rb_define_class_under(mLZMA, "Filter", rb_cObject);
    rb_undef_alloc_func(cFilter);
    rb_define_singleton_method(cFilter, "lzma1", RUBY_METHOD_FUNC(lzma1_new), -1);
    rb_define_singleton_method(cFilter, "lzma2", RUBY_METHOD_FUNC(lzma2_new), -1);
    rb_define_singleton_method(cFilter, "delta", RUBY_METHOD_FUNC(delta_new), -1);

    cBasicLZMA = rb_define_class_under(cFilter, "BasicLZMA", cFilter);

    cLZMA1 = rb_define_class_under(cFilter, "LZMA1", cBasicLZMA);
    rb_define_alloc_func(cLZMA1, lzma1_alloc);
    rb_define_method(cLZMA1, "initialize", lzma1_init, -1);

    cLZMA2 = rb_define_class_under(cFilter, "LZMA2", cBasicLZMA);
    rb_define_alloc_func(cLZMA2, lzma2_alloc);
    rb_define_method(cLZMA2, "initialize", lzma2_init, -1);

    cDelta = rb_define_class_under(cFilter, "Delta", cFilter);
    rb_define_alloc_func(cDelta, delta_alloc);
    rb_define_method(cDelta, "initialize", delta_init, -1);

#define DEF_ACCESSOR(CLASS, NAME)                             \
    rb_define_method(CLASS, #NAME, lzma_get_ ## NAME, 0);     \
    rb_define_method(CLASS, #NAME "=", lzma_set_ ## NAME, 1); \

    DEF_ACCESSOR(cBasicLZMA, dictsize);
    DEF_ACCESSOR(cBasicLZMA, dict);
    DEF_ACCESSOR(cBasicLZMA, lc);
    DEF_ACCESSOR(cBasicLZMA, lp);
    DEF_ACCESSOR(cBasicLZMA, pb);
    DEF_ACCESSOR(cBasicLZMA, mode);
    DEF_ACCESSOR(cBasicLZMA, nice);
    DEF_ACCESSOR(cBasicLZMA, mf);
    DEF_ACCESSOR(cBasicLZMA, depth);

#undef DEF_ACCESSOR
}


// SECTION: LZMA::Stream

static VALUE cStream;
static VALUE cEncoder;
static VALUE cDecoder;
static VALUE cAutoDecoder;
static VALUE cRawEncoder;
static VALUE cRawDecoder;


#define LZMA_TEST(STATUS, ROLLBACK)                             \
    {                                                           \
        lzma_ret status0 = (STATUS);                            \
        if (lzma_isfailed(status0)) {                           \
            { (ROLLBACK); };                                    \
            VALUE exc = lookup_exception(status0);              \
            rb_exc_raise(rb_exc_new3(exc, rb_class_name(exc))); \
        }                                                       \
    }                                                           \

static inline void filter_copy(lzma_filter *dest, VALUE filter)
{
    memcpy(dest, getfilter(filter), sizeof(*dest));
}

static void stream_clear(lzma_stream *stream)
{
    const lzma_stream init = LZMA_STREAM_INIT;
    memcpy(stream, &init, sizeof(init));
}

static inline lzma_stream *getstream(VALUE lzma)
{
    lzma_stream *stream;
    Data_Get_Struct(lzma, lzma_stream, stream);
    return stream;
}

static inline void stream_cleanup(lzma_stream *stream)
{
    lzma_end(stream);
    xfree(stream);
}


static inline int is_sync(size_t sync)
{
    return (sync == LZMA_SYNC_FLUSH || sync == LZMA_FULL_FLUSH || sync == LZMA_FINISH);
}

static VALUE stream_update_0(lzma_stream *stream, lzma_action action, const uint8_t *src, size_t srclen)
{
    return RUBY_BEGIN_BLOCKING_REGION {
        VALUE dest = rb_str_new("", 0);
        uint8_t buf[4 * 1024];
        stream->next_in = src;
        stream->avail_in = srclen;
        while (stream->avail_in > 0 || is_sync(action)) {
            do {
                rb_thread_check_ints();
                stream->next_out = buf;
                stream->avail_out = sizeof(buf);
                lzma_ret status = lzma_code(stream, action);
                if (status == LZMA_STREAM_END) {
                    rb_str_buf_cat(dest, (const char *)buf, stream->next_out - buf);
                    return dest;
                }

                LZMA_TEST(status, {});
                rb_str_buf_cat(dest, (const char *)buf, stream->next_out - buf);
            } while (stream->next_out - buf > 0);
        }
        return dest;
    } RUBY_END_BLOCKING_REGION;
}

/*
 * call-seq:
 *  LZMA::Stream#update(src, flush = LZMA::RUN)
 *
 * データストリームとして渡されたsrcを圧縮/伸張します。
 *
 * [RETURN] Stringインスタンス
 * [EXCEPTION] LZMA::Exceptions::BasicException
 */
static VALUE stream_update(int argc, VALUE argv[], VALUE self)
{
    VALUE src, flush1;
    int flush;
    if (rb_scan_args(argc, argv, "11", &src, &flush1) == 1) {
        flush = LZMA_RUN;
    } else {
        flush = NUM2INT(flush1);
    }
    Check_Type(src, T_STRING);
    src = rb_str_new_frozen(src); // Ruby GVL 領域外でsrcを参照するために独立させる
    lzma_stream *stream = getstream(self);
    return stream_update_0(stream, flush, (const uint8_t *)RSTRING_PTR(src), RSTRING_LEN(src));
}

/*
 * call-seq:
 *  LZMA::Stream#flush(fullsync = false)
 */
static VALUE stream_flush(int argc, VALUE argv[], VALUE self)
{
    VALUE fullsync = Qfalse;
    rb_scan_args(argc, argv, "01", &fullsync);
    lzma_stream *stream = getstream(self);
    size_t sync = RTEST(fullsync) ? LZMA_FULL_FLUSH : LZMA_SYNC_FLUSH;
    return stream_update_0(stream, sync, NULL, 0);
}

/*
 * call-seq:
 *  LZMA::Stream#finish
 */
static VALUE stream_finish(VALUE self)
{
    lzma_stream *stream = getstream(self);
    VALUE dest = stream_update_0(stream, LZMA_FINISH, NULL, 0);

    lzma_end(stream);
    stream_clear(stream);

    return dest;
}



static VALUE stream_alloc(VALUE klass)
{
    lzma_stream *stream;
    VALUE obj = Data_Make_Struct(klass, lzma_stream, NULL, stream_cleanup, stream);
    stream_clear(stream);
    return obj;
}

// filters0はLZMA::Filterクラスのinstanceを与えることができる
static void filter_setup(lzma_filter filter[LZMA_FILTERS_MAX + 1], VALUE filters0[], VALUE *filters0end)
{
    if ((filters0end - filters0) > LZMA_FILTERS_MAX) {
        rb_raise(eFilterTooLong, "filter chain too long (max %d, but given %d)",
                 LZMA_FILTERS_MAX, filters0end - filters0);
    }
    for (; filters0 < filters0end; filters0 ++, filter ++) {
        VALUE f = *filters0;
        if (!rb_obj_is_kind_of(f, cFilter)) {
            rb_raise(rb_eTypeError, "%s", "not a filter");
        }
        filter_copy(filter, f);
    }
    filter->id = LZMA_VLI_UNKNOWN;
    filter->options = NULL;
}


// LZMA::Stream.encoder(filter1, filter2, ...., filterN, check: CRC32)
static VALUE encoder_init(int argc, VALUE argv[], VALUE self)
{
    rb_scan_args(argc, argv, "14", NULL, NULL, NULL, NULL, NULL);
    uint32_t check;
    if (argc > 1 && rb_obj_is_kind_of(argv[argc - 1], rb_cHash)) {
        check = NUM2UINT(argv[argc - 1]);
        argc --;
    } else {
        check = LZMA_CHECK_CRC64;
    }
    lzma_filter filters[LZMA_FILTERS_MAX + 1];
    memset(filters, 0, sizeof(filters));
    filter_setup(filters, argv, argv + argc);

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_stream_encoder(stream, filters, check), {});

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::AutoDecoder.initialize(memlimit = nil, flags = nil)
 */
static VALUE autodecoder_init(int argc, VALUE argv[], VALUE self)
{
    VALUE memlimit0 = Qnil;
    VALUE flags0 = Qnil;
    uint64_t memlimit = UINT64_MAX;
    uint32_t flags = 0;
    rb_scan_args(argc, argv, "02", &memlimit0, &flags0);
    if (!NIL_P(flags0)) { flags = NUM2SIZET(flags0); }
    if (!NIL_P(memlimit0)) { memlimit = NUM2SIZET(memlimit0); }

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_auto_decoder(stream, memlimit, flags), {});

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::Decoder.initialize(memlimit = nil, flags = nil)
 */
static VALUE decoder_init(int argc, VALUE argv[], VALUE self)
{
    VALUE memlimit0 = Qnil;
    VALUE flags0 = Qnil;
    uint64_t memlimit = UINT64_MAX;
    uint32_t flags = 0;
    rb_scan_args(argc, argv, "02", &memlimit0, &flags0);
    if (!NIL_P(flags0)) { flags = NUM2SIZET(flags0); }
    if (!NIL_P(memlimit0)) { memlimit = NUM2SIZET(memlimit0); }

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_stream_decoder(stream, memlimit, flags), {});

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::RawEncoder.initialize(filter1 [ , filter2 [ , .... ] ])
 */
static VALUE rawencoder_init(int argc, VALUE argv[], VALUE self)
{
    if (argc < 1) {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1)", argc);
    }
    lzma_filter filters[LZMA_FILTERS_MAX + 1];
    memset(filters, 0, sizeof(filters));
    filter_setup(filters, argv, argv + argc);

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_raw_encoder(stream, filters), {});

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::RawDecoder.initialize(filter1 [ , filter2 [ , .... ] ])
 */
static VALUE rawdecoder_init(int argc, VALUE argv[], VALUE self)
{
    if (argc < 1) {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1)", argc);
    }
    lzma_filter filters[LZMA_FILTERS_MAX + 1];
    memset(filters, 0, sizeof(filters));
    filter_setup(filters, argv, argv + argc);

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_raw_decoder(stream, filters), {});

    return self;
}

static void setup_stream(void)
{
    cStream = rb_define_class_under(mLZMA, "Stream", rb_cObject);
    rb_undef_alloc_func(cStream);
    rb_define_method(cStream, "update", stream_update, -1);
    rb_define_method(cStream, "flush", stream_flush, -1);
    rb_define_method(cStream, "finish", stream_finish, 0);

    cEncoder = rb_define_class_under(cStream, "Encoder", cStream);
    rb_define_alloc_func(cEncoder, stream_alloc);
    rb_define_method(cEncoder, "initialize", RUBY_METHOD_FUNC(encoder_init), -1);

    cDecoder = rb_define_class_under(cStream, "Decoder", cStream);
    rb_define_alloc_func(cDecoder, stream_alloc);
    rb_define_method(cDecoder, "initialize", RUBY_METHOD_FUNC(decoder_init), -1);

    cAutoDecoder = rb_define_class_under(cStream, "AutoDecoder", cStream);
    rb_define_alloc_func(cAutoDecoder, stream_alloc);
    rb_define_method(cAutoDecoder, "initialize", RUBY_METHOD_FUNC(autodecoder_init), -1);

    cRawEncoder = rb_define_class_under(cStream, "RawEncoder", cStream);
    rb_define_alloc_func(cRawEncoder, stream_alloc);
    rb_define_method(cRawEncoder, "initialize", RUBY_METHOD_FUNC(rawencoder_init), -1);

    cRawDecoder = rb_define_class_under(cStream, "RawDecoder", cStream);
    rb_define_alloc_func(cRawDecoder, stream_alloc);
    rb_define_method(cRawDecoder, "initialize", RUBY_METHOD_FUNC(rawdecoder_init), -1);
}


// SECTION: LZMA::Index


static VALUE cIndex;
static VALUE cIEncoder;
static VALUE cIDecoder;

static VALUE iencoder_alloc(VALUE klass)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE iencoder_init(int argc, VALUE argv[], VALUE self)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE idecoder_alloc(VALUE klass)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE idecoder_init(int argc, VALUE argv[], VALUE self)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}


static void setup_index(void)
{
    cIndex = rb_define_class_under(mLZMA, "Index", rb_cObject);
    rb_undef_alloc_func(cIndex);

    cIEncoder = rb_define_class_under(cIndex, "Encoder", cIndex);
    rb_define_alloc_func(cIEncoder, iencoder_alloc);
    rb_define_method(cIEncoder, "initialize", RUBY_METHOD_FUNC(iencoder_init), -1);

    cIDecoder = rb_define_class_under(cIndex, "Decoder", cIndex);
    rb_define_alloc_func(cIDecoder, idecoder_alloc);
    rb_define_method(cIDecoder, "initialize", RUBY_METHOD_FUNC(idecoder_init), -1);
}


// SECTION: LZMA::Utils

static size_t utils_crc_immediate_max;

/*
 * call-seq:
 *  LZMA::Utils.crc32(binary, crc = 0)
 *
 * liblzmaに含まれるlzma_crc32を呼び出します。
 */
static VALUE utils_crc32(int argc, VALUE argv[], VALUE self)
{
    VALUE src, crc;
    rb_scan_args(argc, argv, "11", &src, &crc);
    if (RSTRING_LEN(src) > utils_crc_immediate_max) {
        src = rb_str_new_frozen(src);

        return RUBY_BEGIN_BLOCKING_REGION {
            uint32_t crc1 = lzma_crc32((const uint8_t *)RSTRING_PTR(src), RSTRING_LEN(src),
                                       NIL_P(crc) ? 0 : NUM2UINT(crc));
            return UINT2NUM(crc1);
        } RUBY_END_BLOCKING_REGION;
    } else {
        uint32_t crc1 = lzma_crc32((const uint8_t *)RSTRING_PTR(src), RSTRING_LEN(src),
                                   NIL_P(crc) ? 0 : NUM2UINT(crc));
        return UINT2NUM(crc1);
    }
}

/*
 * call-seq:
 *  LZMA::Utils.crc64(binary, crc = 0)
 *
 * liblzmaに含まれるlzma_crc64を呼び出します。
 */
static VALUE utils_crc64(int argc, VALUE argv[], VALUE self)
{
    VALUE src, crc;
    rb_scan_args(argc, argv, "11", &src, &crc);
    if (RSTRING_LEN(src) > utils_crc_immediate_max) {
        src = rb_str_new_frozen(src);

        return RUBY_BEGIN_BLOCKING_REGION {
            uint32_t crc1 = lzma_crc64((const uint8_t *)RSTRING_PTR(src), RSTRING_LEN(src),
                                       NIL_P(crc) ? 0 : NUM2UINT(crc));
            return UINT2NUM(crc1);
        } RUBY_END_BLOCKING_REGION;
    } else {
        uint32_t crc1 = lzma_crc64((const uint8_t *)RSTRING_PTR(src), RSTRING_LEN(src),
                                   NIL_P(crc) ? 0 : NUM2UINT(crc));
        return UINT2NUM(crc1);
    }
}

static VALUE mUtils;

void setup_utils(void)
{
    utils_crc_immediate_max = 4 * 1024 * 1024;

    mUtils = rb_define_module_under(mLZMA, "Utils");
    rb_include_module(mLZMA, mUtils);
    rb_extend_object(mLZMA, mUtils);

    // rb_define_module_functionを使わない理由は、rb_define_module_functionで定義すると
    // インスタンスメソッドがprivateで定義されるため、その対策。

    rb_extend_object(mUtils, mUtils);

    rb_define_method(mUtils, "crc32", RUBY_METHOD_FUNC(utils_crc32), -1);
    rb_define_method(mUtils, "crc64", RUBY_METHOD_FUNC(utils_crc64), -1);
}


// SECTION: LZMA

void Init_lzma(void)
{
    symDICTSIZE     = ID2SYM(rb_intern("dict_size"));
    symPRESETDICT   = ID2SYM(rb_intern("preset_dict"));
    symLC           = ID2SYM(rb_intern("lc"));
    symLP           = ID2SYM(rb_intern("lp"));
    symPB           = ID2SYM(rb_intern("pb"));
    symMODE         = ID2SYM(rb_intern("mode"));
    symNICE         = ID2SYM(rb_intern("nice"));
    symMF           = ID2SYM(rb_intern("mf"));
    symDEPTH        = ID2SYM(rb_intern("depth"));
    symCHECK        = ID2SYM(rb_intern("check"));

    mLZMA = rb_define_module("LZMA");

    setup_utils();
    setup_constants();
    setup_exceptions();
    setup_filter();
    setup_stream();
    setup_index();
}
