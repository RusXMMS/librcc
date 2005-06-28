#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "rccconfig.h"
#include "rcclocale.h"

rcc_language_ptr rccGetLanguagePointer(rcc_context ctx, rcc_language_id language_id) {
    if ((!ctx)||(language_id<0)||(language_id>=ctx->n_languages)) return NULL;
    return ctx->languages[language_id];
}

const char *rccGetLanguageName(rcc_context ctx, rcc_language_id language_id) {
    rcc_language_ptr language;
    language = rccGetLanguagePointer(ctx, language_id);
    if (!language) return NULL;
    return language->sn;
}

rcc_language_id rccGetLanguageByName(rcc_context ctx, const char *name) {
    unsigned int i;
    if ((!ctx)||(!name)) return 0;
    
    for (i=0;ctx->languages[i];i++) 
	if (!strcmp(ctx->languages[i]->sn, name)) return i;

    return 0;
}

static rcc_language_id rccGetDefaultLanguage(rcc_context ctx) {
    int err;
    unsigned int i;
    char stmp[RCC_MAX_LANGUAGE_CHARS+1];

    if (!ctx) return -1;
    
    err = rccLocaleGetLanguage(stmp, ctx->locale_variable, RCC_MAX_LANGUAGE_CHARS);
    if (err) {
	if (ctx->n_languages>1) return 1;
	return -1;
    }
    
    for (i=0;ctx->languages[i];i++)
	if (!strcmp(ctx->languages[i]->sn, stmp)) return i;
    
    if (i>1) return 1;
    return -1;
}

rcc_language_id rccGetRealLanguage(rcc_context ctx, rcc_language_id language_id) {
    if ((!ctx)||(language_id<0)||(language_id>=ctx->n_languages)) return -1;
    if (language_id) return language_id;
    return rccGetDefaultLanguage(ctx);    
}

const char *rccGetRealLanguageName(rcc_context ctx, rcc_language_id language_id) {
    language_id = rccGetRealLanguage(ctx, language_id);
    if (language_id<0) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}

rcc_language_id rccGetSelectedLanguage(rcc_context ctx) {
    if (!ctx) return -1;
    return ctx->current_language;
}

const char *rccGetSelectedLanguageName(rcc_context ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetSelectedLanguage(ctx);
    if (language_id<0) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}

rcc_language_id rccGetCurrentLanguage(rcc_context ctx) {
    if (!ctx) return -1;
    return rccGetRealLanguage(ctx, ctx->current_language);    
}

const char *rccGetCurrentLanguageName(rcc_context ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetCurrentLanguage(ctx);
    if (language_id<0) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}


int rccSetLanguage(rcc_context ctx, rcc_language_id language_id) {
    rcc_language_config config;
    
    if ((!ctx)||(language_id < 0)||(language_id >= ctx->n_languages)) return -1;
    if ((!ctx->languages[language_id]->engines[0])||(!ctx->languages[language_id]->charsets[0])) return -2;

    if (ctx->current_language != language_id) {
	config = rccGetConfig(ctx, language_id);
	if (!config) return -1;
	
	ctx->configure = 1;
	ctx->current_language = language_id;
	ctx->current_config = config; 
    }
}

int rccSetLanguageByName(rcc_context ctx, const char *name) {
    rcc_language_id language_id;
    
    language_id = rccGetLanguageByName(ctx, name);
    if (language_id < 0) return -1;
    
    return rccSetLanguage(ctx, language_id);
}
