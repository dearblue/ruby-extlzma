
#include "extlzma.h"

// TODO: 例外を整理して数を減らす。

VALUE extlzma_mExceptions;
VALUE extlzma_eBasicException;
VALUE extlzma_eStreamEnd;
VALUE extlzma_eNoCheck;
VALUE extlzma_eUnsupportedCheck;
VALUE extlzma_eGetCheck;
VALUE extlzma_eMemError;
VALUE extlzma_eMemlimitError;
VALUE extlzma_eFormatError;
VALUE extlzma_eOptionsError;
VALUE extlzma_eDataError;
VALUE extlzma_eBufError;
VALUE extlzma_eProgError;
VALUE extlzma_eFilterTooLong;
VALUE extlzma_eBadPreset;

static inline VALUE
lookup_exception(lzma_ret status)
{
    switch (status) {
    case LZMA_OK:                return Qnil;
    case LZMA_STREAM_END:        return extlzma_eStreamEnd;
    case LZMA_NO_CHECK:          return extlzma_eNoCheck;
    case LZMA_UNSUPPORTED_CHECK: return extlzma_eUnsupportedCheck;
    case LZMA_GET_CHECK:         return extlzma_eGetCheck;
    case LZMA_MEM_ERROR:         return extlzma_eMemError;
    case LZMA_MEMLIMIT_ERROR:    return extlzma_eMemlimitError;
    case LZMA_FORMAT_ERROR:      return extlzma_eFormatError;
    case LZMA_OPTIONS_ERROR:     return extlzma_eOptionsError;
    case LZMA_DATA_ERROR:        return extlzma_eDataError;
    case LZMA_BUF_ERROR:         return extlzma_eBufError;
    case LZMA_PROG_ERROR:        return extlzma_eProgError;
    default:                     return rb_eRuntimeError;
    }
}

VALUE
extlzma_lookup_error(lzma_ret status)
{
    return lookup_exception(status);
}

void
extlzma_init_Exceptions(void)
{
    extlzma_mExceptions = rb_define_module_under(extlzma_mLZMA, "Exceptions");
    rb_include_module(extlzma_mLZMA, extlzma_mExceptions);

    extlzma_eBasicException = rb_define_class_under(extlzma_mExceptions, "BasicException", rb_eStandardError);
    rb_define_class_under(extlzma_mExceptions, "FilterTooLong", extlzma_eBasicException);
    rb_define_class_under(extlzma_mExceptions, "BadPreset", extlzma_eBasicException);

#define DEFINE_EXCEPTION(CLASS, STATUS)                                           \
    {                                                                             \
        extlzma_e ## CLASS =                                                      \
            rb_define_class_under(extlzma_mExceptions,                            \
                #CLASS, extlzma_eBasicException);                                 \
        rb_define_const(extlzma_e ## CLASS, "STATUS", SIZET2NUM(STATUS));         \
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
}
