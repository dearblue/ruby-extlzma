#include "extlzma.h"

VALUE extlzma_cFilter;
static VALUE cBasicLZMA;
static VALUE cLZMA1;
static VALUE cLZMA2;
static VALUE cDelta;

static void *
setup_lzma_preset(size_t preset)
{
    lzma_options_lzma *lzma = xcalloc(sizeof(lzma_options_lzma), 1);
    if (lzma_lzma_preset(lzma, preset)) {
        free(lzma);
        rb_raise(rb_eArgError,
                 "wrong preset level (%u for 0..9) or wrong flag bit(s) (0x%08x)",
                 (unsigned int)(preset & LZMA_PRESET_LEVEL_MASK),
                 (unsigned int)(preset & ~LZMA_PRESET_LEVEL_MASK & ~LZMA_PRESET_EXTREME));
    }
    return (void *)(lzma);
}

/*
 * Document-method: LZMA::Filter::BasicLZMA#dict
 *
 * call-seq:
 * dict -> string or nil
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
 * dict=(dict)
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
 * dictsize -> integer
 *
 * 辞書の大きさをバイト値として取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#dictsize=
 *
 * call-seq:
 * dictsize=(size) -> self
 *
 * 辞書の大きさをバイト値として設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lc
 *
 * call-seq:
 * lc -> integer
 *
 * lc 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lc=
 *
 * call-seq:
 * lc=(value) -> self
 *
 * lc 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lp
 *
 * call-seq:
 * lp -> integer
 *
 * lp 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#lp=
 *
 * call-seq:
 * lp=(value) -> self
 *
 * lp 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#pb
 *
 * call-seq:
 * pb -> integer
 *
 * pb 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#pb=
 *
 * call-seq:
 * pb=(value) -> self
 *
 * pb 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mode
 *
 * call-seq:
 * mode -> integer
 *
 * mode 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mode=
 *
 * call-seq:
 * mode=(value) -> self
 *
 * mode 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#nice
 *
 * call-seq:
 * nice -> integer
 *
 * nice 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#nice=
 *
 * call-seq:
 * nice=(value) -> self
 *
 * nice 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mf
 *
 * call-seq:
 * mf -> integer
 *
 * mf 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#mf=
 *
 * call-seq:
 * mf=(value) -> self
 *
 * mf 値を設定します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#depth
 *
 * call-seq:
 * depth -> integer
 *
 * depth 値を取得します。
 */

/*
 * Document-method: LZMA::Filter::BasicLZMA#depth=
 *
 * call-seq:
 * depth=(value) -> self
 *
 * depth 値を設定します。
 */

#define DEFINE_ACCESSOR_ENTITY(SET0, SET, GET, MEMBER, DEFAULT)                    \
    static inline void                                                             \
    SET0(lzma_options_lzma *filter, VALUE n)                                       \
    {                                                                              \
        if (NIL_P(n)) {                                                            \
            filter->MEMBER = DEFAULT;                                              \
        } else {                                                                   \
            filter->MEMBER = NUM2UINT(n);                                          \
        }                                                                          \
    }                                                                              \
                                                                                   \
    static VALUE                                                                   \
    SET(VALUE self, VALUE n)                                                       \
    {                                                                              \
        SET0((lzma_options_lzma *)extlzma_getfilter(self)->options, n);            \
        return self;                                                               \
    }                                                                              \
                                                                                   \
    static VALUE                                                                   \
    GET(VALUE self)                                                                \
    {                                                                              \
        lzma_options_lzma *p = extlzma_getfilter(self)->options;                   \
        return UINT2NUM(p->MEMBER);                                                \
    }                                                                              \

DEFINE_ACCESSOR_ENTITY(aux_set_dictsize_0, ext_set_dictsize, ext_get_dictsize, dict_size, LZMA_DICT_SIZE_DEFAULT);
DEFINE_ACCESSOR_ENTITY(aux_set_lc_0,       ext_set_lc,       ext_get_lc,       lc,        LZMA_LC_DEFAULT);
DEFINE_ACCESSOR_ENTITY(aux_set_lp_0,       ext_set_lp,       ext_get_lp,       lp,        LZMA_LP_DEFAULT);
DEFINE_ACCESSOR_ENTITY(aux_set_pb_0,       ext_set_pb,       ext_get_pb,       pb,        LZMA_PB_DEFAULT);
DEFINE_ACCESSOR_ENTITY(aux_set_mode_0,     ext_set_mode,     ext_get_mode,     mode,      LZMA_MODE_NORMAL);
DEFINE_ACCESSOR_ENTITY(aux_set_nice_0,     ext_set_nice,     ext_get_nice,     nice_len,  64);
DEFINE_ACCESSOR_ENTITY(aux_set_mf_0,       ext_set_mf,       ext_get_mf,       mf,        LZMA_MF_BT4);
DEFINE_ACCESSOR_ENTITY(aux_set_depth_0,    ext_set_depth,    ext_get_depth,    depth,     0);

#undef DEFINE_ACCESSOR_ENTITY

static ID ivar_id_predict;

static inline void
aux_set_predict_nil(lzma_options_lzma *filter)
{
    filter->preset_dict = NULL;
    filter->preset_dict_size = 0;
}

static inline void
aux_set_predict_string(lzma_options_lzma *filter, VALUE *predict)
{
    *predict = rb_str_new_frozen(*predict);
    filter->preset_dict = (uint8_t *)(RSTRING_PTR(*predict));
    filter->preset_dict_size = RSTRING_LEN(*predict);
}

static inline void
aux_set_predict_0(lzma_options_lzma *filter, VALUE predict, VALUE self)
{
    if (NIL_P(predict)) {
        aux_set_predict_nil(filter);
    } else if (TYPE(predict) == T_STRING) {
        if (RSTRING_LEN(predict) > 0) {
            aux_set_predict_string(filter, &predict);
        } else {
            aux_set_predict_nil(filter);
        }
    } else {
        rb_raise(rb_eTypeError, "%s", "predict is not a String or nil");
    }

    rb_ivar_set(self, ivar_id_predict, predict);
}

static VALUE
ext_set_predict(VALUE self, VALUE predict)
{
    aux_set_predict_0((lzma_options_lzma *)extlzma_getfilter(self)->options, predict, self);
    return self;
}

static VALUE
ext_get_predict(VALUE self)
{
    VALUE predict = rb_attr_get(self, ivar_id_predict);
    if (!NIL_P(predict)) { predict = rb_str_new_shared(predict); }
    return predict;
}

static void *
setup_lzma(VALUE obj, uint32_t preset,
           VALUE dictsize, VALUE predict, VALUE lc, VALUE lp, VALUE pb,
           VALUE mode, VALUE nice, VALUE mf, VALUE depth)
{
    lzma_options_lzma lzma = { 0 };
    if (lzma_lzma_preset(&lzma, preset) != 0) {
        rb_raise(extlzma_eBadPreset, "bad preset (0x%08x)", preset);
    }

    if (RTEST(predict)) { aux_set_predict_0(&lzma, predict, obj); }

#define SETVAR(NAME)                                           \
    if (RTEST(NAME)) { aux_set_ ## NAME ## _0(&lzma, NAME); } \

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
 *  initialize(dist = LZMA::DELTA_DIST_MIN)
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
    lzma_filter *filter = extlzma_getfilter(self);
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
 *  initialize(preset = LZMA::PRESET_DEFAULT, opts = {}) -> filter
 *
 * LZMA フィルタ設定オブジェクトを初期化します。
 *
 * この段階で各値の確認を行うことはせず、*encoderに渡すときに初めて確認されます。
 *
 * [preset = LZMA::PRESET_DEFAULT]
 *      プリセット値 (≒圧縮レベル) を 0-9 の範囲で指定し、任意で LZMA::PRESET_EXTREME を論理和で組み合わせることが出来ます。
 * [opts dictsize]
 *      辞書の大きさをバイト値で指定します。既定値は preset によって変化します。
 * [opts predict: nil]
 *      定義済み辞書を指定します。既定値は nil です。
 * [opts lc: nil]
 *      既定値は preset によって変化します。
 * [opts lp: nil]
 *      既定値は preset によって変化します。
 * [opts pb: nil]
 *      既定値は preset によって変化します。
 * [opts mode: nil]
 *      既定値は preset によって変化します。
 * [opts nice: nil]
 *      既定値は preset によって変化します。
 * [opts mf: nil]
 *      既定値は preset によって変化します。
 * [opts depth: nil]
 *      既定値は preset によって変化します。
 * [RETURN]
 *      フィルタオブジェクト
 * [EXCEPTIONS]
 *      (NO DOCUMENT)
 */
static VALUE
ext_lzma_init(int argc, VALUE argv[], VALUE self)
{
    VALUE preset = Qnil;
    VALUE opts = Qnil;
    rb_scan_args(argc, argv, "01:", &preset, &opts);
    lzma_filter *filter = extlzma_getfilter(self);
    if (NIL_P(opts)) {
        filter->options = setup_lzma_preset(getpreset(preset));
    } else {
        filter->options = setup_lzma(self, getpreset(preset),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_dictsize)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_predict)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_lc)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_lp)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_pb)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_mode)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_nice)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_mf)),
                rb_hash_lookup(opts, ID2SYM(extlzma_id_depth)));
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
 *  dictsize -> 辞書サイズ (バイト長)
 *
 * 様々なフィルタ値の設定・取り出しを行う
 */

void
extlzma_init_Filter(void)
{
    ivar_id_predict = rb_intern_const("extlzma.predict");

    extlzma_cFilter = rb_define_class_under(extlzma_mLZMA, "Filter", rb_cObject);
    rb_undef_alloc_func(extlzma_cFilter);

    cBasicLZMA = rb_define_class_under(extlzma_cFilter, "BasicLZMA", extlzma_cFilter);
    rb_define_method(cBasicLZMA, "initialize", ext_lzma_init, -1);

    cLZMA1 = rb_define_class_under(extlzma_cFilter, "LZMA1", cBasicLZMA);
    rb_define_alloc_func(cLZMA1, lzma1_alloc);

    cLZMA2 = rb_define_class_under(extlzma_cFilter, "LZMA2", cBasicLZMA);
    rb_define_alloc_func(cLZMA2, lzma2_alloc);

    cDelta = rb_define_class_under(extlzma_cFilter, "Delta", extlzma_cFilter);
    rb_define_alloc_func(cDelta, delta_alloc);
    rb_define_method(cDelta, "initialize", delta_init, -1);

    rb_define_method(cBasicLZMA, "dictsize",    ext_get_dictsize, 0);
    rb_define_method(cBasicLZMA, "dictsize=",   ext_set_dictsize, 1);
    rb_define_method(cBasicLZMA, "predict",     ext_get_predict, 0);
    rb_define_method(cBasicLZMA, "predict=",    ext_set_predict, 1);
    rb_define_method(cBasicLZMA, "lc",          ext_get_lc, 0);
    rb_define_method(cBasicLZMA, "lc=",         ext_set_lc, 1);
    rb_define_method(cBasicLZMA, "lp",          ext_get_lp, 0);
    rb_define_method(cBasicLZMA, "lp=",         ext_set_lp, 1);
    rb_define_method(cBasicLZMA, "pb",          ext_get_pb, 0);
    rb_define_method(cBasicLZMA, "pb=",         ext_set_pb, 1);
    rb_define_method(cBasicLZMA, "mode",        ext_get_mode, 0);
    rb_define_method(cBasicLZMA, "mode=",       ext_set_mode, 1);
    rb_define_method(cBasicLZMA, "nice",        ext_get_nice, 0);
    rb_define_method(cBasicLZMA, "nice=",       ext_set_nice, 1);
    rb_define_method(cBasicLZMA, "mf",          ext_get_mf, 0);
    rb_define_method(cBasicLZMA, "mf=",         ext_set_mf, 1);
    rb_define_method(cBasicLZMA, "depth",       ext_get_depth, 0);
    rb_define_method(cBasicLZMA, "depth=",      ext_set_depth, 1);
}
