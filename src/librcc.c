#include <stdio.h>
#include <string.h>

#include <librcd.h>

#include "internal.h"
#include "rccconfig.h"
#include "rccenca.h"
#include "rcclist.h"



int rccInit() {
    /*DS: Load addition languages from config! */
    return rccEncaInit();
}

void rccFree() {
    rccEncaFree();
}

rcc_context rccCreateContext(const char *locale_variable, unsigned int max_languages, unsigned int max_classes, rcc_class_ptr defclasses, rcc_init_flags flags) {
    int err;
    unsigned int i;
    
    rcc_context ctx;
    rcc_language_ptr *languages;
    rcc_class_ptr *classes;
    rcc_language_config configs;
    iconv_t *from, *to;

    if (!max_languages) {
	if (flags&RCC_NO_DEFAULT_CONFIGURATION) max_languages = RCC_MAX_LANGUAGES;
	else {
	    for (i=0;rcc_default_languages[i].sn;i++);
	    max_languages = i;
	}
    }
    
    if (!max_classes) {
	if (defclasses) {
	    for (i=0;defclasses[i].name;i++);
	    max_classes = i;
	} else max_classes = RCC_MAX_CLASSES;
    }

    ctx = (rcc_context)malloc(sizeof(struct rcc_context_t));
    languages = (rcc_language_ptr*)malloc((max_languages+1)*sizeof(rcc_language_ptr));
    classes = (rcc_class_ptr*)malloc((max_classes+1)*sizeof(rcc_class_ptr));
    from = (iconv_t*)malloc((max_classes)*sizeof(iconv_t));
    to = (iconv_t*)malloc((max_classes)*sizeof(iconv_t));

    configs = (rcc_language_config)malloc((max_languages)*sizeof(struct rcc_language_config_t));
    
    if ((!ctx)||(!languages)||(!classes)) {
	if (from) free(from);
	if (to) free(to);
	if (configs) free(configs);
	if (classes) free(classes);
	if (languages) free(languages);
	if (ctx) free(ctx);
	return NULL;
    }

    ctx->configuration_lock = 0;

    ctx->aliases[0] = NULL;
    for (i=0;rcc_default_aliases[i].alias;i++)
	rccRegisterLanguageAlias(ctx, rcc_default_aliases + i);

    ctx->languages = languages;
    ctx->max_languages = max_languages;
    ctx->n_languages = 0;
    languages[0] = NULL;

    ctx->classes = classes;
    ctx->max_classes = max_classes;
    ctx->n_classes = 0;
    classes[0] = NULL;

    ctx->fsiconv = (iconv_t)-1;
    
    ctx->iconv_from = from;
    ctx->iconv_to = to;
    for (i=0;i<max_classes;i++) {
	from[i] = (iconv_t)-1;
	to[i] = (iconv_t)-1;
    }
    
    for (i=0;i<RCC_MAX_CHARSETS;i++)
	ctx->iconv_auto[i] = (iconv_t)-1;
    
    ctx->configs = configs;
    for (i=0;i<max_languages;i++)
	configs[i].charset = NULL;

    err = rccEngineInit(&ctx->engine_ctx, ctx);
    if (err) {
	rccFree(ctx);
	return NULL;
    }
    
    ctx->current_language = 0;

    if (locale_variable) {
	if (strlen(locale_variable)>=RCC_MAX_VARIABLE_CHARS) {
	    rccFree(ctx);
	    return NULL;
	}
	strcpy(ctx->locale_variable, locale_variable);
    } else {
	strcpy(ctx->locale_variable, RCC_LOCALE_VARIABLE);
    }
    
    if (flags&RCC_NO_DEFAULT_CONFIGURATION) {
	rccRegisterLanguage(ctx, rcc_default_languages);
	ctx->current_config = NULL;
    } else {
	for (i=0;rcc_default_languages[i].sn;i++)
	    rccRegisterLanguage(ctx, rcc_default_languages+i);
	
	if (max_languages < i) {
	    rccFree(ctx);
	    return NULL;
	}

	ctx->current_config = rccGetCurrentConfig(ctx);
    } 
    
    if (defclasses) {
	for (i=0;defclasses[i].name;i++)
	    rccRegisterClass(ctx, defclasses+i);

	if (max_classes < i) {
	    rccFree(ctx);
	    return NULL;
	}	    
    }

    for (i=0;i<RCC_MAX_OPTIONS;i++)
	ctx->options[i] = 0;    

    ctx->configure = 1;
    
    return ctx;
}

static void rccFreeIConv(rcc_context ctx) {
    unsigned int i;
    
    if ((!ctx)||(!ctx->iconv_from)||(!ctx->iconv_to)) return;

    if ((ctx->fsiconv != (iconv_t)-1)&&(ctx->fsiconv != (iconv_t)-2)) {
	iconv_close(ctx->fsiconv);
	ctx->fsiconv = (iconv_t)-1;
    }
    
    for (i=0;i<ctx->n_classes;i++) {
	if ((ctx->iconv_from[i] != (iconv_t)-1)&&(ctx->iconv_from[i] != (iconv_t)-2)) {
	    iconv_close(ctx->iconv_from[i]);
	    ctx->iconv_from[i] = (iconv_t)-1;
	}
	if ((ctx->iconv_to[i] != (iconv_t)-1)&&(ctx->iconv_to[i] != (iconv_t)-2)) {
	    iconv_close(ctx->iconv_to[i]);
	    ctx->iconv_to[i] = (iconv_t)-1;
	}
    }
    for (i=0;i<RCC_MAX_CHARSETS;i++) {
	if ((ctx->iconv_auto[i] != (iconv_t)-1)&&(ctx->iconv_auto[i] != (iconv_t)-2)) {
	    iconv_close(ctx->iconv_auto[i]);
	    ctx->iconv_auto[i] = (iconv_t)-1;
	}
    }    
}

void rccFreeContext(rcc_context ctx) {
    unsigned int i;
    
    if (ctx) {
	rccEngineFree(&ctx->engine_ctx);
	rccFreeIConv(ctx);
	if (ctx->iconv_from) free(ctx->iconv_from);
	if (ctx->iconv_to) free(ctx->iconv_to);
	
	if (ctx->configs) {
	    for (i=0;i<ctx->max_languages;i++)
		rccConfigFree(ctx->configs+i);
	    free(ctx->configs);
	}
	if (ctx->classes) free(ctx->classes);
	if (ctx->languages) free(ctx->languages);
	free(ctx);
    }
}

int rccLockConfiguration(rcc_context ctx, unsigned int lock_code) {
    if (!ctx) return -1;
    if (ctx->configuration_lock) return -3;
    ctx->configuration_lock = lock_code;
    return 0;
}

int rccUnlockConfiguration(rcc_context ctx, unsigned int lock_code) {
    if (!ctx) return -1;
    if (ctx->configuration_lock != lock_code) return -3;
    ctx->configuration_lock = 0;
    return 0;    
}

rcc_language_id rccRegisterLanguage(rcc_context ctx, rcc_language *language) {
    if ((!ctx)||(!language)) return -1;
    if (ctx->configuration_lock) return -3;
    
    if (ctx->n_languages == ctx->max_languages) return -2;
    ctx->languages[ctx->n_languages++] = language;
    ctx->languages[ctx->n_languages] = NULL;
    
    if (!ctx->current_language)
	ctx->current_config = rccGetCurrentConfig(ctx);
    
    return ctx->n_languages-1;
}

rcc_charset_id rccLanguageRegisterCharset(rcc_language *language, rcc_charset charset) {
    unsigned int i;
    
    if ((!language)||(!charset)) return -1;
    for (i=0;language->charsets[i];i++);
    if (i>=RCC_MAX_CHARSETS) return -2;
    language->charsets[i++] = charset;
    language->charsets[i] = NULL;
    return i-1;
}

rcc_engine_id rccLanguageRegisterEngine(rcc_language *language, rcc_engine *engine) {
    unsigned int i;
    
    if ((!language)||(!engine)) return -1;
    for (i=0;language->engines[i];i++);
    if (i>=RCC_MAX_ENGINES) return -2;
    language->engines[i++] = engine;
    language->engines[i] = NULL;
    return i-1;
}

rcc_alias_id rccRegisterLanguageAlias(rcc_context ctx, rcc_language_alias *alias) {
    unsigned int i;
    
    if ((!ctx)||(!alias)) return -1;
    
    for (i=0;ctx->aliases[i];i++)
    if (i>=RCC_MAX_ALIASES) return -2;
    
    ctx->aliases[i++] = alias;
    ctx->aliases[i] = NULL;

    return i-1;
}

rcc_class_id rccRegisterClass(rcc_context ctx, rcc_class *cl) {
    puts("yes");
    if ((!ctx)||(!cl)) return -1;
    if (ctx->configuration_lock) return -3;
    if (ctx->n_classes == ctx->max_classes) return -2;

    puts(" -----> New class");
    ctx->configure = 1;
    ctx->classes[ctx->n_classes++] = cl;
    ctx->classes[ctx->n_classes] = NULL;
    return ctx->n_classes-1;
}


rcc_class_type rccGetClassType(rcc_context ctx, rcc_class_id class_id) {
    if ((!ctx)||(class_id<0)||(class_id>=ctx->n_classes)) return RCC_CLASS_INVALID;
    
    return ctx->classes[class_id]->class_type;
}


int rccConfigure(rcc_context ctx) {
    unsigned int i;
    rcc_charset *charsets;
    const char *charset;
    
    if (!ctx) return -1;
    if (!ctx->configure) return 0;
    
    rccFreeIConv(ctx);
    for (i=0;i<ctx->n_classes;i++) {
	charset = rccGetCurrentCharsetName(ctx, i);
	if (strcmp(charset, "UTF-8")&&strcmp(charset, "UTF8")) {
	    ctx->iconv_from[i] = iconv_open("UTF-8", charset);
	    ctx->iconv_to[i] = iconv_open(charset, "UTF-8");
	} else {
	    ctx->iconv_from[i] = (iconv_t)-2;
	    ctx->iconv_to[i] = (iconv_t)-2;
	}
    }

    charsets = rccGetCurrentAutoCharsetList(ctx);
    for (i=0;charsets[i];i++) {
	charset = charsets[i];
	if (strcmp(charset, "UTF-8")&&strcmp(charset, "UTF8"))
	    ctx->iconv_auto[i] = iconv_open("UTF-8", charset);
	else
	    ctx->iconv_auto[i] = (iconv_t)-2;
    }
    
    rccEngineConfigure(&ctx->engine_ctx);
    
    return 0;
}

char *rccCreateResult(rcc_context ctx, int len, int *rlen) {
    char *res;

    if (!len) len = strlen(ctx->tmpbuffer);

    res = (char*)malloc(len+1);
    if (!res) return NULL;

    memcpy(res, ctx->tmpbuffer, len);
    res[len] = 0;
    
    if (rlen) *rlen = len;
    
    return res;    
}
