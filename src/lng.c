#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <librcd.h>
#include "librcc.h"

const char *rccGetLanguageName(rcc_context *ctx, rcc_language_id language_id) {
    if ((!ctx)||(language_id<0)||(language_id>=ctx->n_languages)) return NULL;
    return ctx->languages[language_id]->sn;
}

language_id rccGetLanguageByName(rcc_context *ctx, const char *name) {
    unsigned int i;
    if ((!ctx)||(!name)) return 0;
    
    for (i=0;ctx->languages[i];i++) 
	if (!strcmp(ctx->languages[i]->sn, name)) return i;

    return 0;
}

static int rccGetLocaleLanguage(char *result, const char *lv, unsigned int n) {
    charset_list_t *enc;
    char *l;
    
    if (!lv) return -1;

    l = setlocale(lv, NULL);
    if (!l) return -1;
    else if ((strcmp(l,"C")==0)||(strcmp(l,"POSIX")==0)) return -1;

    for (i=0;((l[i])&&(l[i]!='.'));i++);

    for (i=0;rcc_default_aliases[i].alias;i++) 
	if (strncmp(l,rcc_default_aliases[i].alias,i)==0) {
	    l = rcc_default_aliases[i].alias;
	    break;
	}

    for (i=0;((l[i])&&(l[i]!='.')&&(l[i]!='_'));i++);
    if (i>=n) return -1;

    strncpy(result,l,i);
    result[i]=0;

    return 0;
}

static rcc_language_id rccGetDefaultLanguage(rc_context *ctx) {
    int err;
    unsigned int i;
    char stmp[RCC_MAX_LANGUAGE_CHARS+1];

    if (!ctx) return -1;
    
    err = rccGetLocaleLanguage(stmp, ctx->locale_variable, RCC_MAX_LANGUAGE_CHARS);
    if (err) {
	if (ctx->n_languages>1) return 1;
	return -1;
    }
    
    for (i=0;ctx->languages[i];i++)
	if (!strcmp(ctx->languages[i]->sn, stmp)) return i;
    
    if (i>1) return 1;
    return -1;
}

rcc_language_id rccGetRealLanguage(rcc_context *ctx, rcc_language_id language_id) {
    if ((!ctx)||(language_id<0)||(language_id>=ctx->n_languages)) return -1;
    if (language_id) return language_id;
    return rccGetDefaultLanguage(ctx);    
}

const char *rccGetRealLanguageName(rcc_context *ctx, rcc_language_id language_id) {
    language_id = rccGetRealLanguage(ctx, language_id);
    if (language_id<0) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}

rcc_language_id rccGetSelectedLanguage(rcc_context *ctx) {
    if (!ctx) return NULL;
    return ctx->current_language;
}

const char *rccGetSelectedLanguageName(rcc_context *ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetSelectedLanguage(ctx);
    if (language_id<0) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}

rcc_language_id rccGetCurrentLanguage(rcc_context *ctx) {
    if (!ctx) return -1;
    return rccGetRealLanguage(ctx, ctx->current_language);    
}

const char *rccGetCurrentLanguageName(rcc_context *ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetCurrentLanguage(ctx);
    if (language_id<0) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}


int rccSetLanguage(rcc_context *ctx, rcc_language_id language_id) {
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

int rccSetLanguageByName(rcc_context *ctx, const char *name) {
    rcc_language_id language_id;
    
    language_id = rccGetLanguageByName(ctx, name);
    if (language_id < 0) return -1;
    
    return rccSetLanguage(ctx, language_id);
}
