#ifndef EXTLZMA_H
#define EXTLZMA_H 1

#include <stdarg.h>
#include <lzma.h>
#include <ruby.h>
#include <ruby/thread.h>
#include <ruby/intern.h>

#define LOG { fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __func__); }

#define LOGF(FORMAT, ...)                                           \
    {                                                               \
        fprintf(stderr, "%s:%d:%s: " FORMAT "\n",                   \
                __FILE__, __LINE__, __func__, ## __VA_ARGS__);      \
    }                                                               \

#define ELEMENTOF(VECT) (sizeof(VECT) / sizeof((VECT)[0]))

#define AUX_FUNCALL(RECV, METHOD, ...)                  \
    ({                                                  \
        VALUE args__aux_funcall__[] = { __VA_ARGS__ };  \
        rb_funcall2((RECV), (METHOD),                   \
                ELEMENTOF(args__aux_funcall__),         \
                args__aux_funcall__);                   \
    })                                                  \

extern VALUE extlzma_mLZMA;
extern VALUE extlzma_cFilter;
extern VALUE extlzma_cStream;
extern VALUE extlzma_mExceptions;

extern VALUE extlzma_eBasicException;
extern VALUE extlzma_eStreamEnd;
extern VALUE extlzma_eNoCheck;
extern VALUE extlzma_eUnsupportedCheck;
extern VALUE extlzma_eGetCheck;
extern VALUE extlzma_eMemError;
extern VALUE extlzma_eMemlimitError;
extern VALUE extlzma_eFormatError;
extern VALUE extlzma_eOptionsError;
extern VALUE extlzma_eDataError;
extern VALUE extlzma_eBufError;
extern VALUE extlzma_eProgError;
extern VALUE extlzma_eFilterTooLong;
extern VALUE extlzma_eBadPreset;

extern ID extlzma_id_dictsize;
extern ID extlzma_id_predict;
extern ID extlzma_id_lc;
extern ID extlzma_id_lp;
extern ID extlzma_id_pb;
extern ID extlzma_id_mode;
extern ID extlzma_id_nice;
extern ID extlzma_id_mf;
extern ID extlzma_id_depth;
extern ID extlzma_id_check;
extern ID extlzma_id_none;
extern ID extlzma_id_crc32;
extern ID extlzma_id_crc64;
extern ID extlzma_id_sha256;

extern void extlzma_init_Stream(void);
extern void extlzma_init_Utils(void);
extern void extlzma_init_Constants(void);
extern void extlzma_init_Exceptions(void);
extern void extlzma_init_Filter(void);
extern void extlzma_init_Index(void);
extern VALUE extlzma_lookup_error(lzma_ret status);

static inline int
aux_lzma_isfailed(lzma_ret status)
{
    return status != 0;
}

static inline void
aux_lzma_test(lzma_ret status)
{
    if (aux_lzma_isfailed(status)) {
        VALUE exc = extlzma_lookup_error(status);
        exc = rb_exc_new2(exc, "failed lzma processing");
        rb_exc_raise(exc);
    }
}

#define AUX_LZMA_TEST(STATUS) aux_lzma_test((STATUS))

static inline void *
checkref(VALUE obj, void *p)
{
    if (!p) {
        rb_raise(rb_eArgError,
                 "not initialized yet - #<%s:%p>",
                 rb_obj_classname(obj), (void *)obj);
    }

    return p;
}

static inline void *
check_notref(VALUE obj, void *p)
{
    if (p) {
        rb_raise(rb_eArgError,
                 "already initialized - #<%s:%p>",
                 rb_obj_classname(obj), (void *)obj);
    }

    return p;
}

static inline void *
getrefp(VALUE obj)
{
    void *p;
    Data_Get_Struct(obj, void, p);
    return p;
}

static inline void *
getref(VALUE obj)
{
    return checkref(obj, getrefp(obj));
}

static inline lzma_filter *
getfilterp(VALUE obj)
{
    return getrefp(obj);
}

static inline lzma_filter *
extlzma_getfilter(VALUE obj)
{
    return getref(obj);
}

// rdoc に対して定義されているものと錯覚させるマクロ
#define RDOCFAKE(...)

#define EXTLZMA_NOT_REACHABLE                \
    rb_bug("NOT REACHABLE HERE - %s:%d(%s)", \
           __FILE__, __LINE__, __func__);    \


static inline void *
aux_thread_call_without_gvl_main(void *pp)
{
    va_list *p = (va_list *)pp;
    void *(*func)(void *) = va_arg(*p, void *(*)(void *));
    va_list *va2 = va_arg(*p, va_list *);
    return func(va2);
}

static inline void *
aux_thread_call_without_gvl_dummy(void *dummy, ...)
{
    va_list va;
    va_start(va, dummy);
    void *p = rb_thread_call_without_gvl(
            aux_thread_call_without_gvl_main, &va, RUBY_UBF_PROCESS, 0);
    va_end(va);
    return p;
}

static inline void *
aux_thread_call_without_gvl(void *(*func)(va_list *), ...)
{
    va_list va;
    va_start(va, func);
    void *p = aux_thread_call_without_gvl_dummy(0, func, &va);
    va_end(va);
    return p;
}

#endif /* EXTLZMA_H */
