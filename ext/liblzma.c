/* encoding:utf-8 */

/*
 * liblzma.c -
 * - Author:    dearblue <dearblue@sourceforge.jp>
 * - Copyright: Copyright (c) 2010 dearblue
 * - License:   Distributed under the 2-clause BSD License
 */

#include <ruby.h>
#include <ruby/intern.h>
#include <lzma.h>


static VALUE mLZMA;


static VALUE symDICTSIZE;
static VALUE symDICT;
static VALUE symLC;
static VALUE symLP;
static VALUE symPB;
static VALUE symMODE;
static VALUE symNICE;
static VALUE symMF;
static VALUE symDEPTH;
static VALUE symCHECK;


// rdoc に対して定義されているものと錯覚させるマクロ
#define RDOC(...)


static inline int
lzma_isfailed(lzma_ret status)
{
    return status != 0;
}

#define LZMA_TEST(STATUS)                                       \
    {                                                           \
        lzma_ret _status = (STATUS);                            \
        if (lzma_isfailed(_status)) {                           \
            VALUE exc = lookup_exception(_status);              \
            rb_exc_raise(rb_exc_new3(exc, rb_class_name(exc))); \
        }                                                       \
    }                                                           \


// SECTION: LZMA::Constants

static VALUE mConstants;

static void
setup_constants(void)
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

    DEFINE_CONSTANT(PRESET_DEFAULT,     UINT2NUM(LZMA_PRESET_DEFAULT));
    DEFINE_CONSTANT(PRESET_LEVEL_MASK,  UINT2NUM(LZMA_PRESET_LEVEL_MASK));
    DEFINE_CONSTANT(PRESET_EXTREME,     UINT2NUM(LZMA_PRESET_EXTREME));
    DEFINE_CONSTANT(DICT_SIZE_MIN,      UINT2NUM(LZMA_DICT_SIZE_MIN));
    DEFINE_CONSTANT(DICT_SIZE_DEFAULT,  UINT2NUM(LZMA_DICT_SIZE_DEFAULT));
    DEFINE_CONSTANT(LCLP_MIN,           UINT2NUM(LZMA_LCLP_MIN));
    DEFINE_CONSTANT(LCLP_MAX,           UINT2NUM(LZMA_LCLP_MAX));
    DEFINE_CONSTANT(LC_DEFAULT,         UINT2NUM(LZMA_LC_DEFAULT));
    DEFINE_CONSTANT(LP_DEFAULT,         UINT2NUM(LZMA_LP_DEFAULT));
    DEFINE_CONSTANT(PB_MIN,             UINT2NUM(LZMA_PB_MIN));
    DEFINE_CONSTANT(PB_MAX,             UINT2NUM(LZMA_PB_MAX));
    DEFINE_CONSTANT(PB_DEFAULT,         UINT2NUM(LZMA_PB_DEFAULT));
    DEFINE_CONSTANT(MODE_FAST,          UINT2NUM(LZMA_MODE_FAST));
    DEFINE_CONSTANT(MODE_NORMAL,        UINT2NUM(LZMA_MODE_NORMAL));
    DEFINE_CONSTANT(MF_HC3,             UINT2NUM(LZMA_MF_HC3));
    DEFINE_CONSTANT(MF_HC4,             UINT2NUM(LZMA_MF_HC4));
    DEFINE_CONSTANT(MF_BT2,             UINT2NUM(LZMA_MF_BT2));
    DEFINE_CONSTANT(MF_BT3,             UINT2NUM(LZMA_MF_BT3));
    DEFINE_CONSTANT(MF_BT4,             UINT2NUM(LZMA_MF_BT4));

    DEFINE_CONSTANT(CHECK_NONE,         UINT2NUM(LZMA_CHECK_NONE));
    DEFINE_CONSTANT(CHECK_CRC32,        UINT2NUM(LZMA_CHECK_CRC32));
    DEFINE_CONSTANT(CHECK_CRC64,        UINT2NUM(LZMA_CHECK_CRC64));
    DEFINE_CONSTANT(CHECK_SHA256,       UINT2NUM(LZMA_CHECK_SHA256));
                                       
    DEFINE_CONSTANT(RUN,                UINT2NUM(LZMA_RUN));
    DEFINE_CONSTANT(FULL_FLUSH,         UINT2NUM(LZMA_FULL_FLUSH));
    DEFINE_CONSTANT(SYNC_FLUSH,         UINT2NUM(LZMA_SYNC_FLUSH));
    DEFINE_CONSTANT(FINISH,             UINT2NUM(LZMA_FINISH));

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

static inline VALUE
lookup_exception(lzma_ret status)
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

static void
setup_exceptions(void)
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


static inline lzma_filter *
getfilter(VALUE obj)
{
    lzma_filter *filter;
    Data_Get_Struct(obj, lzma_filter, filter);
    return filter;
}

static void *
setup_lzma_preset(size_t preset)
{
    lzma_options_lzma *lzma = xcalloc(sizeof(lzma_options_lzma), 1);
    if (lzma_lzma_preset(lzma, preset)) {
        rb_raise(rb_eArgError,
                 "wrong preset level (%d for 0..9) or wrong flag bit(s) (0x%08x)",
                 preset & LZMA_PRESET_LEVEL_MASK,
                 preset & ~LZMA_PRESET_LEVEL_MASK & ~LZMA_PRESET_EXTREME);
    }
    return (void *)(lzma);
}


/*
 * Document-method: LZMA::Filter::BasicLZMA#dict
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#dict -> string or nil
 *
 * 定義済み辞書を取得します。
 *
 * [RETURN] 定義済み辞書が定義されている場合は文字列が返ります。定義されていなければ +nil+ が返ります。
 *
 *          エンコード情報は無視されます (常に Encoding::BINARY として扱われます)。呼び出し側で統一しておくべきです。
 *
 *          返される文字列は定義辞書の複成体です。変更しても定義情報には反映されません。+#dict=+ と併せて利用して下さい。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#dict=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#dict=(dict)
 *
 * 定義済み辞書を定義します。
 *
 * [dict]   定義済み辞書として文字列を与えます。定義を無効にする (定義済み辞書を利用しない) には、+nil+ を与えます。
 *
 *          エンコード情報は無視されます (常に Encoding::BINARY として扱われます)。呼び出し側で統一しておくべきです。
 *
 *          与えた文字列は内部で複写され、元の文字列とは独立します。文字列の変更を反映したい場合はその都度 +#dict=+ を呼ぶ必要があります。
 *
 * [RETURN] 自身 (self) を返します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#dictsize
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#dictsize -> integer
 *
 * 辞書の大きさをバイト値として取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#dictsize=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#dictsize=(size) -> self
 *
 * 辞書の大きさをバイト値として設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lc
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#lc -> integer
 *
 * lc 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lc=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#lc=(value) -> self
 *
 * lc 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lp
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#lp -> integer
 *
 * lp 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lp=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#lp=(value) -> self
 *
 * lp 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#pb
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#pb -> integer
 *
 * pb 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#pb=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#pb=(value) -> self
 *
 * pb 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mode
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#mode -> integer
 *
 * mode 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mode=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#mode=(value) -> self
 *
 * mode 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#nice
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#nice -> integer
 *
 * nice 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#nice=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#nice=(value) -> self
 *
 * nice 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mf
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#mf -> integer
 *
 * mf 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mf=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#mf=(value) -> self
 *
 * mf 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#depth
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#depth -> integer
 *
 * depth 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#depth=
 *
 * call-seq:
 * LZMA::Filter::BasicLZMA#depth=(value) -> self
 *
 * depth 値を設定します。
 */


#define DEFINE_ACCESSOR_ENTITY(NAME, MEMBER, DEFAULT)                              \
    static inline void                                                             \
    lzma_set_ ## NAME ## _0(lzma_options_lzma *filter, VALUE n)                    \
    {                                                                              \
        if (NIL_P(n)) {                                                            \
            filter->MEMBER = DEFAULT;                                              \
        } else {                                                                   \
            filter->MEMBER = NUM2UINT(n);                                          \
        }                                                                          \
    }                                                                              \
                                                                                   \
    static VALUE                                                                   \
    lzma_set_ ## NAME(VALUE self, VALUE n)                                         \
    {                                                                              \
        lzma_set_ ## NAME ## _0((lzma_options_lzma *)getfilter(self)->options, n); \
        return self;                                                               \
    }                                                                              \
                                                                                   \
    static VALUE                                                                   \
    lzma_get_ ## NAME(VALUE self)                                                  \
    {                                                                              \
        return UINT2NUM(((lzma_options_lzma *)getfilter(self)->options)->MEMBER);  \
    }                                                                              \

static ID IDdictpreset;

static inline void
lzma_set_dict_nil(lzma_options_lzma *filter)
{
    filter->preset_dict = NULL;
    filter->preset_dict_size = 0;
}

static inline void
lzma_set_dict_string(lzma_options_lzma *filter, VALUE dict)
{
    dict = rb_str_new_frozen(dict);
    filter->preset_dict = (uint8_t *)(RSTRING_PTR(dict));
    filter->preset_dict_size = RSTRING_LEN(dict);
}

static inline void
lzma_set_dict_0(lzma_options_lzma *filter, VALUE dict, VALUE self)
{
    if (NIL_P(dict)) {
        lzma_set_dict_nil(filter);
    } else if (TYPE(dict) == T_STRING) {
        if (RSTRING_LEN(dict) > 0) {
            lzma_set_dict_string(filter, dict);
        } else {
            lzma_set_dict_nil(filter);
        }
    } else {
        rb_raise(rb_eTypeError, "%s", "dict is not a String or nil");
    }

    rb_ivar_set(self, IDdictpreset, dict);
}

static VALUE
lzma_set_dict(VALUE self, VALUE dict)
{
    lzma_set_dict_0((lzma_options_lzma *)getfilter(self)->options, dict, self);
    return self;
}

static VALUE
lzma_get_dict(VALUE self)
{
    VALUE dict = rb_attr_get(self, IDdictpreset);
    if (!NIL_P(dict)) { dict = rb_str_new_shared(dict); }
    return dict;
}

DEFINE_ACCESSOR_ENTITY(dictsize,   dict_size,   LZMA_DICT_SIZE_DEFAULT); // lzma_set_dictsize_0, lzma_set_dictsize, lzma_set_lc
DEFINE_ACCESSOR_ENTITY(lc,         lc,          LZMA_LC_DEFAULT);
DEFINE_ACCESSOR_ENTITY(lp,         lp,          LZMA_LP_DEFAULT);
DEFINE_ACCESSOR_ENTITY(pb,         pb,          LZMA_PB_DEFAULT);
DEFINE_ACCESSOR_ENTITY(mode,       mode,        LZMA_MODE_NORMAL);
DEFINE_ACCESSOR_ENTITY(nice,       nice_len,    64);
DEFINE_ACCESSOR_ENTITY(mf,         mf,          LZMA_MF_BT4);
DEFINE_ACCESSOR_ENTITY(depth,      depth,       0);

#undef DEFINE_ACCESSOR_ENTITY


static void *
setup_lzma(VALUE obj, uint32_t preset,
           VALUE dictsize, VALUE dictpreset, VALUE lc, VALUE lp, VALUE pb,
           VALUE mode, VALUE nice, VALUE mf, VALUE depth)
{
    lzma_options_lzma lzma = { 0 };
    if (lzma_lzma_preset(&lzma, preset) != 0) {
        rb_raise(eBadPreset, "bad preset (0x%08x)", preset);
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

static void
cleanup_filter(lzma_filter *filter)
{
    if (filter->options) { xfree(filter->options); }
    xfree(filter);
}


static uint32_t
getpreset(VALUE preset)
{
    if (NIL_P(preset)) {
        return LZMA_PRESET_DEFAULT;
    } else {
        return NUM2UINT(preset);
    }
}

/*
 * call-seq:
 *  LZMA::Filter.lzma1(...)
 *
 * LZMA::Filter::LZMA1.newを呼ぶための補助機構です。
 *
 * LZMA::Filter::LZMA1クラスの .new や #initialize を書き換えた場合はそれに従います
 * (あくまで .new を呼ぶだけです)。
 */
static VALUE
lzma1_new(int argc, VALUE argv[], VALUE self)
{
    return rb_class_new_instance(argc, argv, cLZMA1);
}

/*
 * call-seq:
 *  LZMA::Filter.lzma2(...)
 *
 * LZMA::Filter::LZMA2.new を呼ぶための補助機構です。
 *
 * LZMA::Filter::LZMA2クラスの .new や #initialize を書き換えた場合はそれに従います
 * (あくまで .new を呼ぶだけです)。
 */
static VALUE
lzma2_new(int argc, VALUE argv[], VALUE self)
{
    return rb_class_new_instance(argc, argv, cLZMA2);
}

/*
 * call-seq:
 *  LZMA::Filter.delta(...)
 *
 * LZMA::Filter::Delta.new を呼ぶための補助機構です。
 *
 * LZMA::Filter::Deltaクラスの .new や #initialize を書き換えた場合はそれに従います
 * (あくまで .new を呼ぶだけです)。
 */
static VALUE
delta_new(int argc, VALUE argv[], VALUE self)
{
    return rb_class_new_instance(argc, argv, cDelta);
}


static inline VALUE
filter_alloc(VALUE klass, lzma_vli id)
{
    lzma_filter *filter;
    VALUE obj = Data_Make_Struct(klass, lzma_filter, NULL, cleanup_filter, filter);
    memset(filter, 0, sizeof(*filter));
    filter->id = id;
    return obj;
}

static VALUE
lzma1_alloc(VALUE klass)
{
    return filter_alloc(klass, LZMA_FILTER_LZMA1);
}

static VALUE
lzma2_alloc(VALUE klass)
{
    return filter_alloc(klass, LZMA_FILTER_LZMA2);
}

static VALUE
delta_alloc(VALUE klass)
{
    return filter_alloc(cDelta, LZMA_FILTER_DELTA);
}

/*
 * call-seq:
 *  LZMA::Filter::Delta.new(dist = LZMA::DELTA_DIST_MIN)
 *
 * 差分フィルタ設定オブジェクトを返します。
 *
 * distは1要素あたりのバイト長で、1以上255以下を指定できます。
 *
 * NOTE::
 *  使用する場合多くの場合1で十分と思われますが、音楽CDの音声データであれば1サンプル2バイトであるため2が有効でしょう。
 *
 *  しかし元のデータによっては圧縮効率を低下させることがあるため、実際に適用するべきかはデータの特性によって検証するのが好ましいです。
 */
static VALUE
delta_init(int argc, VALUE argv[], VALUE self)
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

/*
 * call-seq:
 *  LZMA::Filter::BasicLZMA.new(preset = LZMA::PRESET_DEFAULT, ... ) -> filter
 *
 * LZMA フィルタ設定オブジェクトを初期化します。
 *
 * この段階で各値の確認を行うことはせず、*encoderに渡すときに初めて確認されます。
 *
 * [preset]     プリセット値 (≒圧縮レベル) を 0-9 の範囲で指定し、任意で LZMA::PRESET_EXTREME を論理和で組み合わせることが出来ます。既定値はLZMA::PRESET_DEFAULTとなります。
 * [dictsize]   辞書の大きさをバイト値で指定します。既定値は preset によって変化します。
 * [dict]       定義済み辞書を指定します。既定値は nil です。
 * [lc]         既定値は preset によって変化します。
 * [lp]         既定値は preset によって変化します。
 * [pb]         既定値は preset によって変化します。
 * [mode]       既定値は preset によって変化します。
 * [nice]       既定値は preset によって変化します。
 * [mf]         既定値は preset によって変化します。
 * [depth]      既定値は preset によって変化します。
 * [RETURN]     フィルタオブジェクト
 * [EXCEPTIONS] (NO DOCUMENT)
 */
static VALUE
lzma_init(int argc, VALUE argv[], VALUE self)
{
    VALUE preset = Qnil;
    VALUE opts = Qnil;
    rb_scan_args(argc, argv, "01:", &preset, &opts);
    lzma_filter *filter = getfilter(self);
    if (NIL_P(opts)) {
        filter->options = setup_lzma_preset(getpreset(preset));
    } else {
        filter->options = setup_lzma(self, getpreset(preset),
                                     rb_hash_lookup(opts, symDICTSIZE),
                                     rb_hash_lookup(opts, symDICT),
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
 * Document-class: LZMA::Filter
 *
 * LZMA::Filter クラスは、各圧縮器や伸張器の生成時に用いるフィルタ情報を取り扱います。
 *
 * liblzma で定義されているフィルタはそれぞれ LZMA::Filter::LZMA1 /
 * LZMA::Filter::LZMA2 / LZMA::Filter::Delta として定義されています。
 *
 * これらのクラスについてはそれぞれの文書を見てください。
 */

/*
 * Document-class: LZMA::Filter::BasicLZMA
 *
 * LZMA::Filter::LZMA1 と LZMA::Filter::LZMA2 の基本となるクラスです。
 *
 * allocator を持たないため、このクラス自身はインスタンスを作成することが出来ません。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#dictsize -> integer
 *
 * call-seq:
 *  LZMA::Filter::BasicLZMA#dictsize -> 辞書サイズ (バイト長)
 *
 * 様々なフィルタ値の設定・取り出しを行う
 */

static void
setup_filter(void)
{
    cFilter = rb_define_class_under(mLZMA, "Filter", rb_cObject);
    rb_undef_alloc_func(cFilter);
    rb_define_singleton_method(cFilter, "lzma1", RUBY_METHOD_FUNC(lzma1_new), -1);
    rb_define_singleton_method(cFilter, "lzma2", RUBY_METHOD_FUNC(lzma2_new), -1);
    rb_define_singleton_method(cFilter, "delta", RUBY_METHOD_FUNC(delta_new), -1);

    cBasicLZMA = rb_define_class_under(cFilter, "BasicLZMA", cFilter);
    rb_define_method(cBasicLZMA, "initialize", lzma_init, -1);

    cLZMA1 = rb_define_class_under(cFilter, "LZMA1", cBasicLZMA);
    rb_define_alloc_func(cLZMA1, lzma1_alloc);

    cLZMA2 = rb_define_class_under(cFilter, "LZMA2", cBasicLZMA);
    rb_define_alloc_func(cLZMA2, lzma2_alloc);

    cDelta = rb_define_class_under(cFilter, "Delta", cFilter);
    rb_define_alloc_func(cDelta, delta_alloc);
    rb_define_method(cDelta, "initialize", delta_init, -1);


    rb_define_method(cBasicLZMA, "dictsize",    lzma_get_dictsize, 0);
    rb_define_method(cBasicLZMA, "dictsize=",   lzma_set_dictsize, 1);
    rb_define_method(cBasicLZMA, "dict",        lzma_get_dict, 0);
    rb_define_method(cBasicLZMA, "dict=",       lzma_set_dict, 1);
    rb_define_method(cBasicLZMA, "lc",          lzma_get_lc, 0);
    rb_define_method(cBasicLZMA, "lc=",         lzma_set_lc, 1);
    rb_define_method(cBasicLZMA, "lp",          lzma_get_lp, 0);
    rb_define_method(cBasicLZMA, "lp=",         lzma_set_lp, 1);
    rb_define_method(cBasicLZMA, "pb",          lzma_get_pb, 0);
    rb_define_method(cBasicLZMA, "pb=",         lzma_set_pb, 1);
    rb_define_method(cBasicLZMA, "mode",        lzma_get_mode, 0);
    rb_define_method(cBasicLZMA, "mode=",       lzma_set_mode, 1);
    rb_define_method(cBasicLZMA, "nice",        lzma_get_nice, 0);
    rb_define_method(cBasicLZMA, "nice=",       lzma_set_nice, 1);
    rb_define_method(cBasicLZMA, "mf",          lzma_get_mf, 0);
    rb_define_method(cBasicLZMA, "mf=",         lzma_set_mf, 1);
    rb_define_method(cBasicLZMA, "depth",       lzma_get_depth, 0);
    rb_define_method(cBasicLZMA, "depth=",      lzma_set_depth, 1);
}


// SECTION: LZMA::Stream

static VALUE cStream;
static VALUE cEncoder;
static VALUE cDecoder;
static VALUE cAutoDecoder;
static VALUE cRawEncoder;
static VALUE cRawDecoder;


static inline void
filter_copy(lzma_filter *dest, VALUE filter)
{
    memcpy(dest, getfilter(filter), sizeof(*dest));
}

static void
stream_clear(lzma_stream *stream)
{
    const lzma_stream init = LZMA_STREAM_INIT;
    memcpy(stream, &init, sizeof(init));
}

static inline lzma_stream *
getstream(VALUE lzma)
{
    lzma_stream *stream;
    Data_Get_Struct(lzma, lzma_stream, stream);
    return stream;
}

static inline void
stream_cleanup(lzma_stream *stream)
{
    lzma_end(stream);
    xfree(stream);
}


static inline int
is_sync(size_t sync)
{
    return (sync == LZMA_SYNC_FLUSH || sync == LZMA_FULL_FLUSH || sync == LZMA_FINISH);
}


struct stream_update_args
{
    lzma_stream *stream;
    lzma_action action;
    const uint8_t *src;
    size_t srclen;
};

static VALUE
stream_update_1(struct stream_update_args *args)
{
    VALUE dest = rb_str_new("", 0);
    uint8_t buf[4 * 1024];
    args->stream->next_in = args->src;
    args->stream->avail_in = args->srclen;
    while (args->stream->avail_in > 0 || is_sync(args->action)) {
        do {
            rb_thread_check_ints();
            args->stream->next_out = buf;
            args->stream->avail_out = sizeof(buf);
            lzma_ret status = lzma_code(args->stream, args->action);
            if (status == LZMA_STREAM_END) {
                rb_str_buf_cat(dest, (const char *)buf, args->stream->next_out - buf);
                return dest;
            }

            LZMA_TEST(status);
            rb_str_buf_cat(dest, (const char *)buf, args->stream->next_out - buf);
        } while (args->stream->next_out - buf > 0);
    }
    return dest;
}

static inline VALUE
stream_update_2(struct stream_update_args *args)
{
    return rb_thread_blocking_region((VALUE (*)(void *))stream_update_1, args, RUBY_UBF_IO, NULL);
}

static inline VALUE
stream_update_0(lzma_stream *stream, lzma_action action, VALUE src)
{
    if (RTEST(src)) {
        StringValue(src);
        rb_str_locktmp(src);
        struct stream_update_args args = {
            stream, action,
            (const uint8_t *)RSTRING_PTR(src), RSTRING_LEN(src),
        };
        return rb_ensure(stream_update_2, (VALUE)&args, rb_str_unlocktmp, src);
    } else {
        struct stream_update_args args = {
            stream, action, NULL, 0,
        };
        return stream_update_2(&args);
    }
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
static VALUE
stream_update(int argc, VALUE argv[], VALUE self)
{
    VALUE src, flush1;
    int flush;
    if (rb_scan_args(argc, argv, "11", &src, &flush1) == 1) {
        flush = LZMA_RUN;
    } else {
        flush = NUM2INT(flush1);
    }
    lzma_stream *stream = getstream(self);
    return stream_update_0(stream, flush, src);
}

/*
 * call-seq:
 *  LZMA::Stream#flush(fullsync = false)
 */
static VALUE
stream_flush(int argc, VALUE argv[], VALUE self)
{
    VALUE fullsync = Qfalse;
    rb_scan_args(argc, argv, "01", &fullsync);
    lzma_stream *stream = getstream(self);
    size_t sync = RTEST(fullsync) ? LZMA_FULL_FLUSH : LZMA_SYNC_FLUSH;
    return stream_update_0(stream, sync, Qnil);
}

/*
 * call-seq:
 *  LZMA::Stream#finish
 */
static VALUE
stream_finish(VALUE self)
{
    lzma_stream *stream = getstream(self);
    VALUE dest = stream_update_0(stream, LZMA_FINISH, Qnil);

    lzma_end(stream);
    stream_clear(stream);

    return dest;
}



static VALUE
stream_alloc(VALUE klass)
{
    lzma_stream *stream;
    VALUE obj = Data_Make_Struct(klass, lzma_stream, NULL, stream_cleanup, stream);
    stream_clear(stream);
    return obj;
}

// filters0はLZMA::Filterクラスのinstanceを与えることができる
static void
filter_setup(lzma_filter filter[LZMA_FILTERS_MAX + 1], VALUE filters0[], VALUE *filters0end)
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


/*
 * call-seq:
 *  LZMA::Stream::Encoder.new(filter1 [ , filter2 [ , filter3 [ , filter4 ] ] ], check: CHECK_CRC64) -> encoder
 * 
 * 圧縮器を生成します。圧縮されたデータストリームは xz ファイルフォーマットです。
 *
 * [filter1, filter2, filter3, filter4] LZMA::Filter インスタンス。最低一つを必要とします。
 * [check] チェックメソッド。CHECK_NONE CHECK_CRC32 CHECK_CRC64 CHECK_SHA256 のいずれかの定数を与えます。
 * [RETURN] 生成された圧縮器
 * [EXCEPTIONS] (NO DOCUMENTS)
 */
static VALUE
encoder_init(int argc, VALUE argv[], VALUE self)
{
    VALUE vcheck;
    rb_scan_args(argc, argv, "13:", NULL, NULL, NULL, NULL, &vcheck);
    uint32_t check;
    if (NIL_P(vcheck)) {
        check = LZMA_CHECK_CRC64;
    } else {
        check = NUM2UINT(vcheck);
        argc --;
    }
    lzma_filter filters[LZMA_FILTERS_MAX + 1];
    memset(filters, 0, sizeof(filters));
    filter_setup(filters, argv, argv + argc);

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_stream_encoder(stream, filters, check));

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::AutoDecoder.new(memlimit = nil, flags = nil)
 */
static VALUE
autodecoder_init(int argc, VALUE argv[], VALUE self)
{
    VALUE memlimit0 = Qnil;
    VALUE flags0 = Qnil;
    uint64_t memlimit = UINT64_MAX;
    uint32_t flags = 0;
    rb_scan_args(argc, argv, "02", &memlimit0, &flags0);
    if (!NIL_P(flags0)) { flags = NUM2SIZET(flags0); }
    if (!NIL_P(memlimit0)) { memlimit = NUM2SIZET(memlimit0); }

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_auto_decoder(stream, memlimit, flags));

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::Decoder.new(memlimit = nil, flags = nil)
 */
static VALUE
decoder_init(int argc, VALUE argv[], VALUE self)
{
    VALUE memlimit0 = Qnil;
    VALUE flags0 = Qnil;
    uint64_t memlimit = UINT64_MAX;
    uint32_t flags = 0;
    rb_scan_args(argc, argv, "02", &memlimit0, &flags0);
    if (!NIL_P(flags0)) { flags = NUM2SIZET(flags0); }
    if (!NIL_P(memlimit0)) { memlimit = NUM2SIZET(memlimit0); }

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_stream_decoder(stream, memlimit, flags));

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::RawEncoder.new(filter1 [ , filter2 [ , .... ] ]) -> encoder
 *
 * 生の (xzヘッダなどの付かない) LZMA1/2ストリームを構成する圧縮器を生成する。
 *
 * filterは1つ以上4つまでを与える。
 */
static VALUE
rawencoder_init(int argc, VALUE argv[], VALUE self)
{
    if (argc < 1 || argc > 4) {
        rb_scan_args(argc, argv, "13", NULL, NULL, NULL, NULL);
    }
    lzma_filter filters[LZMA_FILTERS_MAX + 1];
    memset(filters, 0, sizeof(filters));
    filter_setup(filters, argv, argv + argc);

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_raw_encoder(stream, filters));

    return self;
}

/*
 * call-seq:
 *  LZMA::Stream::RawDecoder.new(filter1 [ , filter2 [ , .... ] ])
 */
static VALUE
rawdecoder_init(int argc, VALUE argv[], VALUE self)
{
    if (argc < 1 || argc > 4) {
        rb_scan_args(argc, argv, "13", NULL, NULL, NULL, NULL);
    }
    lzma_filter filters[LZMA_FILTERS_MAX + 1];
    memset(filters, 0, sizeof(filters));
    filter_setup(filters, argv, argv + argc);

    lzma_stream *stream = getstream(self);
    LZMA_TEST(lzma_raw_decoder(stream, filters));

    return self;
}

static void
setup_stream(void)
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

static VALUE
iencoder_alloc(VALUE klass)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE
iencoder_init(int argc, VALUE argv[], VALUE self)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE
idecoder_alloc(VALUE klass)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}

static VALUE
idecoder_init(int argc, VALUE argv[], VALUE self)
{
    rb_raise(rb_eNotImpError, "%s", "IMPLEMENT ME!");
}


static void
setup_index(void)
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

struct crc_args
{
    const uint8_t *ptr;
    size_t len;
    VALUE crc;
    VALUE (*crc_update)(struct crc_args *args);
};

static VALUE
crc_calc_try(struct crc_args *args)
{
    return rb_thread_blocking_region((VALUE (*)(void *))(args->crc_update), args, RUBY_UBF_IO, NULL);
}

static inline VALUE
crc_calc(VALUE (*crc_update)(struct crc_args *args), int argc, VALUE argv[])
{
    VALUE src, crc;
    rb_scan_args(argc, argv, "11", &src, &crc);
    StringValue(src);
    rb_str_locktmp(src);

    struct crc_args args = {
        (const uint8_t *)RSTRING_PTR(src),
        RSTRING_LEN(src),
        NIL_P(crc) ? INT2FIX(0) : crc,
        crc_update,
    };

    return rb_ensure(crc_calc_try, (VALUE)&args, rb_str_unlocktmp, src);
}


static VALUE
crc32_update(struct crc_args *args)
{
    return UINT2NUM(lzma_crc32(args->ptr, args->len, NUM2UINT(args->crc)));
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
    return crc_calc(crc32_update, argc, argv);
}


static VALUE
crc64_update(struct crc_args *args)
{
    return ULL2NUM(lzma_crc64(args->ptr, args->len, NUM2ULL(args->crc)));
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
    return crc_calc(crc64_update, argc, argv);
}


static VALUE mUtils;

static void
setup_utils(void)
{
    mUtils = rb_define_module_under(mLZMA, "Utils");

    rb_extend_object(mLZMA, mUtils);
    rb_extend_object(mUtils, mUtils); // 自分に対してもモジュールメソッドを利用できるようにする

    rb_define_method(mUtils, "crc32", RUBY_METHOD_FUNC(utils_crc32), -1);
    rb_define_method(mUtils, "crc64", RUBY_METHOD_FUNC(utils_crc64), -1);
}

// SECTION: LibLZMA

void
Init_liblzma(void)
{
    symDICTSIZE     = ID2SYM(rb_intern("dictsize"));
    symDICT         = ID2SYM(rb_intern("dict"));
    symLC           = ID2SYM(rb_intern("lc"));
    symLP           = ID2SYM(rb_intern("lp"));
    symPB           = ID2SYM(rb_intern("pb"));
    symMODE         = ID2SYM(rb_intern("mode"));
    symNICE         = ID2SYM(rb_intern("nice"));
    symMF           = ID2SYM(rb_intern("mf"));
    symDEPTH        = ID2SYM(rb_intern("depth"));
    symCHECK        = ID2SYM(rb_intern("check"));

    IDdictpreset = rb_intern_const("liblzma.dict_preset");


    mLZMA = rb_define_module("LibLZMA");

    setup_utils();
    setup_constants();
    setup_exceptions();
    setup_filter();
    setup_stream();
//    setup_index();
}
