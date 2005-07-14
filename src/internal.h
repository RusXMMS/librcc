#ifndef _RCC_INTERNAL_H
#define _RCC_INTERNAL_H

#ifndef LIBRCC_DATA_DIR
# define LIBRCC_DATA_DIR "/usr/lib/rcc"
#endif /* LIBRCC_DATA_DIR */

#include "librcc.h"
#include "recode.h"
#include "engine.h"
#include "lngconfig.h"
#include "rccstring.h"
#include "rccdb4.h"
#include "rcciconv.h"

#define STRNLEN(str,n) (n?strnlen(str,n):strlen(str))

#define RCC_MAX_PLUGINS 32
#define RCC_MAX_STRING_CHARS 1024
#define RCC_MAX_PREFIX_CHARS 32

struct rcc_context_t {
    char locale_variable[RCC_MAX_VARIABLE_CHARS+1];
    
    rcc_language_alias_list aliases;

    rcc_option_value options[RCC_MAX_OPTIONS];
    unsigned char default_options[RCC_MAX_OPTIONS];
     
    unsigned int max_languages;
    unsigned int n_languages;
    rcc_language_ptr *languages;
    rcc_language_config configs;
    
    unsigned int max_classes;
    unsigned int n_classes;
    rcc_class_ptr *classes;

    rcc_engine_context_s engine_ctx;

    rcc_iconv *iconv_from;
    rcc_iconv iconv_auto[RCC_MAX_CHARSETS];

    rcc_iconv fsiconv;

    char tmpbuffer[RCC_MAX_STRING_CHARS+sizeof(rcc_string_header)+1];
    char lastprefix[RCC_MAX_PREFIX_CHARS+1];
    
    unsigned char configure;
    rcc_language_config current_config;
    rcc_language_id current_language;
    
    rcc_language_id default_language;

    db4_context db4ctx;
    
    unsigned int configuration_lock;
};
typedef struct rcc_context_t rcc_context_s;

int rccConfigure(rcc_context ctx);
char *rccCreateResult(rcc_context ctx, size_t len, size_t *rlen);

extern rcc_context rcc_default_ctx;
extern char *rcc_home_dir;

#endif /* _RCC_INTERNAL_H */
