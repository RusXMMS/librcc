#ifndef _RCC_ENGINE_H
#define _RCC_ENGINE_H

#include "../config.h"

#ifdef HAVE_RCD
# define RCC_RCD_SUPPORT
# undef RCC_RCD_DYNAMIC
#elif HAVE_DLOPEN
# define RCC_RCD_SUPPORT
# define RCC_RCD_DYNAMIC
#else
# undef RCC_RCD_SUPPORT
# undef RCC_RCD_DYNAMIC
#endif

#define RCC_RCD_LIB "librcd.so.0"

#ifdef RCC_RCD_DYNAMIC
# define RCC_RCD_SUPPORT
#endif

struct rcc_engine_context_t {
    rcc_context ctx;
    rcc_language *language;
    
    rcc_engine_function func;
    rcc_engine_free_function free_func;

    rcc_engine_internal internal;
};
typedef struct rcc_engine_context_t rcc_engine_context_s;

int rccEngineInit();
void rccEngineFree();

int rccEngineInitContext(rcc_engine_context engine_ctx, rcc_context ctx);
void rccEngineFreeContext(rcc_engine_context engine_ctx);

rcc_charset_id rccAutoengineRussian(rcc_engine_context ctx, const char *buf, int len);

#endif /* _RCC_ENGINE_H */
