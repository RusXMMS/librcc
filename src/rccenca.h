#ifndef _RCC_ENCA_H
#define _RCC_ENCA_H

#include "../config.h"

#ifdef HAVE_ENCA
# define RCC_ENCA_SUPPORT
# undef RCC_ENCA_DYNAMIC
#elif HAVE_DLOPEN
# define RCC_ENCA_SUPPORT
# define RCC_ENCA_DYNAMIC
#else
# undef RCC_ENCA_SUPPORT
# undef RCC_ENCA_DYNAMIC
#endif

#define RCC_ENCA_LIB "libenca.so.0"

#ifdef RCC_ENCA_DYNAMIC
# define RCC_ENCA_SUPPORT
#endif

struct rcc_enca_corrections_t {
    char *lang;
    const char *enca_charset;
    const char *iconv_charset;
};
typedef struct rcc_enca_corrections_t rcc_enca_corrections;


int rccEncaInit();
void rccEncaFree();

#endif /* _RCC_ENCA_H */
