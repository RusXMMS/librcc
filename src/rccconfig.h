#ifndef _RCC_CONFIG_H
#define _RCC_CONFIG_H
#include "librcc.h"
#include "opt.h"

#undef RCC_DEBUG
#undef RCC_DEBUG_LANGDETECT
#define RCC_LOCALE_VARIABLE "LC_CTYPE"

extern const char rcc_default_language_sn[];
extern const char rcc_english_language_sn[];
extern const char rcc_disabled_language_sn[];
extern const char rcc_disabled_engine_sn[];

extern rcc_language_alias rcc_default_aliases[];
extern rcc_language_alias rcc_default_aliases_embeded[];
extern rcc_language_relation rcc_default_relations[];
extern rcc_language_relation rcc_default_relations_embeded[];
extern const char rcc_default_charset[];
extern const char rcc_utf8_charset[];

extern const char rcc_engine_nonconfigured[];
extern const char rcc_option_nonconfigured[];

extern rcc_engine rcc_default_engine;
extern rcc_engine rcc_russian_engine;
extern rcc_engine rcc_ukrainian_engine;

extern rcc_language rcc_default_languages_embeded[];
extern rcc_language rcc_default_languages[];
extern rcc_option_description rcc_option_descriptions_embeded[];
extern rcc_option_description rcc_option_descriptions[];

rcc_option_description *rccGetOptionDescription(rcc_option option);
rcc_option_description *rccGetOptionDescriptionByName(const char *name);

rcc_language_id rccDefaultGetLanguageByName(const char *name);
unsigned int rccDefaultDropLanguageRelations(const char *lang);

int rccIsUTF8(const char *name);

#endif /* _RCC_CONFIG_H */
