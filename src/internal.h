#ifndef _RCC_INTERNAL_H
#define _RCC_INTERNAL_H

#include <iconv.h>
#include "librcc.h"
#include "recode.h"
#include "engine.h"
#include "lngconfig.h"

struct rcc_context_t {
    char locale_variable[RCC_MAX_VARIABLE_CHARS+1];
    
    rcc_language_alias_list aliases;
     
    unsigned int max_languages;
    unsigned int n_languages;
    rcc_language_ptr *languages;
    rcc_language_config configs;
    
    unsigned int max_classes;
    unsigned int n_classes;
    rcc_class_ptr *classes;

    rcc_engine_context_s engine_ctx;

    iconv_t *iconv_from;
    iconv_t *iconv_to;
    iconv_t iconv_auto[RCC_MAX_CHARSETS];

    char tmpbuffer[RCC_MAX_STRING_CHARS+sizeof(rcc_string_header)+1];
    iconv_t fsiconv;
    
    unsigned char configure;
    rcc_language_config current_config;
    rcc_language_id current_language;
};
typedef struct rcc_context_t rcc_context_s;

int rccConfigure(rcc_context ctx);
char *rccCreateResult(rcc_context ctx, int len, int *rlen);

#endif /* _RCC_INTERNAL_H */
