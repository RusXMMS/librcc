#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "rccconfig.h"
#include "rcclocale.h"

rcc_language_ptr rccGetLanguagePointer(rcc_context ctx, rcc_language_id language_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if (language_id>=ctx->n_languages) return NULL;
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

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_language_id)-1;
    }
    if (!name) return (rcc_language_id)-1;

    for (i=0;ctx->languages[i];i++)
	if (!strcasecmp(ctx->languages[i]->sn, name)) return (rcc_language_id)i;

    return (rcc_language_id)-1;
}

static rcc_language_id rccGetDefaultLanguage(rcc_context ctx) {
    unsigned int i;
    rcc_option_value clo;
    rcc_engine_ptr *engines;
    rcc_language_config config;
    char stmp[RCC_MAX_LANGUAGE_CHARS+1];

    if (ctx->default_language) return ctx->default_language;
    
    if (!rccLocaleGetLanguage(stmp, ctx->locale_variable, RCC_MAX_LANGUAGE_CHARS)) {
    	for (i=0;ctx->languages[i];i++) {
	    if (!strcmp(ctx->languages[i]->sn, stmp)) {
		clo = rccGetOption(ctx, RCC_OPTION_CONFIGURED_LANGUAGES_ONLY);
		if (clo) {
		    config = rccCheckConfig(ctx, (rcc_language_id)i);
		    if ((!config)||(!config->configured)) {
			if (clo == 1) {
			    engines = ctx->languages[i]->engines;
			    if ((!engines[0])||(!engines[1])) break;
			} else break;
		    }
		}
		ctx->default_language = (rcc_language_id)i;
		return (rcc_language_id)i;
	    }
	}
    }
    
    if (ctx->n_languages>1) return (rcc_language_id)1;
    return (rcc_language_id)-1;
}

rcc_language_id rccGetRealLanguage(rcc_context ctx, rcc_language_id language_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_language_id)-1;
    }
    if (language_id>=ctx->n_languages) return (rcc_language_id)-1;

    if (language_id) return language_id;
    return rccGetDefaultLanguage(ctx);    
}

const char *rccGetRealLanguageName(rcc_context ctx, rcc_language_id language_id) {
    language_id = rccGetRealLanguage(ctx, language_id);
    if (language_id == (rcc_language_id)-1) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}

rcc_language_id rccGetSelectedLanguage(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_language_id)-1;
    }

    return ctx->current_language;
}

const char *rccGetSelectedLanguageName(rcc_context ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetSelectedLanguage(ctx);
    if (language_id == (rcc_language_id)-1) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}

rcc_language_id rccGetCurrentLanguage(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_language_id)-1;
    }
    
    return rccGetRealLanguage(ctx, ctx->current_language);    
}

const char *rccGetCurrentLanguageName(rcc_context ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetCurrentLanguage(ctx);
    if (language_id == (rcc_language_id)-1) return NULL;
    
    return rccGetLanguageName(ctx, language_id);
}


int rccSetLanguage(rcc_context ctx, rcc_language_id language_id) {
    rcc_language_config config;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    
    if (language_id >= ctx->n_languages) return -1;
    if ((!ctx->languages[language_id]->engines[0])||(!ctx->languages[language_id]->charsets[0])) return -2;

    if (ctx->current_language != language_id) {
	config = rccGetConfig(ctx, language_id);
	// NULL is Okey (Off), if (!config) return -1;
	
	ctx->configure = 1;
	ctx->current_language = language_id;
	ctx->current_config = config; 
    }

    return 0;
}

int rccSetLanguageByName(rcc_context ctx, const char *name) {
    rcc_language_id language_id;
    
    language_id = rccGetLanguageByName(ctx, name);
    if (language_id == (rcc_language_id)-1) return -1;
    
    return rccSetLanguage(ctx, language_id);
}
