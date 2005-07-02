#ifndef _RCC_CONFIG_H
#define _RCC_CONFIG_H
#include "librcc.h"

#undef RCC_DEBUG
#define RCC_LOCALE_VARIABLE "LC_CTYPE"

extern rcc_language_alias rcc_default_aliases[];

extern const char rcc_engine_nonconfigured[];

extern rcc_engine rcc_default_engine;
extern rcc_engine rcc_russian_engine;

extern rcc_language rcc_default_languages[];

extern rcc_language_name rcc_default_language_names[];
extern rcc_option_name rcc_default_option_names[];

#endif /* _RCC_CONFIG_H */
