
#include "extlzma.h"

// SECTION: LZMA::Constants

static VALUE mConstants;

void
extlzma_init_Constants(void)
{
    mConstants = rb_define_module_under(extlzma_mLZMA, "Constants");
    rb_include_module(extlzma_mLZMA, mConstants);

    // TODO: マクロを展開した状態で記述する。
    // TODO: 目的は rdoc や yard で文書化できるようにするため。

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

    DEFINE_CONSTANT(OK,                 UINT2NUM(LZMA_OK));
    DEFINE_CONSTANT(STREAM_END,         UINT2NUM(LZMA_STREAM_END));
    DEFINE_CONSTANT(NO_CHECK,           UINT2NUM(LZMA_NO_CHECK));
    DEFINE_CONSTANT(UNSUPPORTED_CHECK,  UINT2NUM(LZMA_UNSUPPORTED_CHECK));
    DEFINE_CONSTANT(GET_CHECK,          UINT2NUM(LZMA_GET_CHECK));
    DEFINE_CONSTANT(MEM_ERROR,          UINT2NUM(LZMA_MEM_ERROR));
    DEFINE_CONSTANT(MEMLIMIT_ERROR,     UINT2NUM(LZMA_MEMLIMIT_ERROR));
    DEFINE_CONSTANT(FORMAT_ERROR,       UINT2NUM(LZMA_FORMAT_ERROR));
    DEFINE_CONSTANT(OPTIONS_ERROR,      UINT2NUM(LZMA_OPTIONS_ERROR));
    DEFINE_CONSTANT(DATA_ERROR,         UINT2NUM(LZMA_DATA_ERROR));
    DEFINE_CONSTANT(BUF_ERROR,          UINT2NUM(LZMA_BUF_ERROR));
    DEFINE_CONSTANT(PROG_ERROR,         UINT2NUM(LZMA_PROG_ERROR));
}
