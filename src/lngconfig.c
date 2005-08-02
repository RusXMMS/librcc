#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#include "internal.h"
#include "rccconfig.h"
#include "rcclocale.h"
#include "lng.h"

rcc_engine_ptr rccConfigGetEnginePointer(rcc_language_config config, rcc_engine_id engine_id) {
    unsigned int i;
    rcc_engine_ptr *engines;

    if ((!config)||(!config->language)) return NULL;
    if (engine_id == (rcc_engine_id)-1) return NULL;
    
    engines = config->language->engines;
    for (i=0;engines[i];i++);
    if (engine_id>=i) return NULL;
    
    return engines[engine_id];
}

rcc_engine_ptr rccConfigCheckEnginePointer(rcc_language_config config, rcc_engine_id engine_id) {
    rcc_engine_ptr engine;
    
    engine = rccConfigGetEnginePointer(config, engine_id);
    if ((engine)||(engine->func)) return engine;
    return NULL;
}

rcc_engine_ptr rccConfigGetCurrentEnginePointer(rcc_language_config config) {
    rcc_engine_id engine_id;

    engine_id = rccConfigGetCurrentEngine(config);
    if (engine_id == (rcc_engine_id)-1) return NULL;

    return rccConfigGetEnginePointer(config, engine_id);
}

rcc_engine_ptr rccConfigCheckCurrentEnginePointer(rcc_language_config config) {
    rcc_engine_ptr engine;
    
    engine = rccConfigGetCurrentEnginePointer(config);
    if ((engine)||(engine->func)) return engine;
    return NULL;
}


const char *rccConfigGetEngineName(rcc_language_config config, rcc_engine_id engine_id) {
    rcc_engine_ptr engine;

    if ((!config)||(!config->language)) return NULL;
    
    if (engine_id == (rcc_engine_id)-1) return rcc_engine_nonconfigured;

    engine = rccConfigGetEnginePointer(config, engine_id);
    if (!engine) return NULL;

    return engine->title;
}

const char *rccConfigGetCharsetName(rcc_language_config config, rcc_charset_id charset_id) {
    unsigned int i;
    rcc_charset *charsets;
    
    if ((!config)||(!config->language)) return NULL;

    charsets = config->language->charsets;
    
    for (i=0;charsets[i];i++);
    if (charset_id>=i) return NULL;
    
    return charsets[charset_id];
}

const char *rccConfigGetAutoCharsetName(rcc_language_config config, rcc_autocharset_id charset_id) {
    unsigned int i;
    rcc_engine_id engine_id;
    rcc_charset *charsets;
    rcc_engine_ptr *engines;
    
    if ((!config)||(!config->language)) return NULL;
    engine_id = rccConfigGetCurrentEngine(config);
    if (engine_id == (rcc_engine_id)-1) return NULL;

    engines = config->language->engines;
    charsets = engines[engine_id]->charsets;

    for (i=0;charsets[i];i++);
    if (charset_id>=i) return NULL;
    
    return charsets[charset_id];
}


rcc_engine_id rccConfigGetEngineByName(rcc_language_config config, const char *name) {
    unsigned int i;
    rcc_engine **engines;

    if ((!config)||(!config->language)||(!name)) return (rcc_engine_id)-1;
    
    engines = config->language->engines;
    for (i=0;engines[i];i++)
	if (!strcasecmp(engines[i]->title,name)) return (rcc_engine_id)i;
    
    return (rcc_engine_id)-1;
}

rcc_charset_id rccConfigGetCharsetByName(rcc_language_config config, const char *name) {
    unsigned int i;
    rcc_charset *charsets;

    if ((!config)||(!config->language)||(!name)) return (rcc_charset_id)-1;
    
    charsets = config->language->charsets;
    for (i=0;charsets[i];i++)
	if (!strcasecmp(charsets[i],name)) return (rcc_charset_id)i;
    
    return (rcc_charset_id)-1;
}

rcc_autocharset_id rccConfigGetAutoCharsetByName(rcc_language_config config, const char *name) {
    unsigned int i;
    rcc_engine_id engine_id;
    rcc_charset *charsets;
    rcc_engine_ptr *engines;

    if ((!config)||(!config->language)||(!name)) return (rcc_autocharset_id)-1;
    
    engine_id = rccConfigGetCurrentEngine(config);
    if (engine_id == (rcc_engine_id)-1) return (rcc_autocharset_id)-1;

    engines = config->language->engines;
    charsets = engines[engine_id]->charsets;
    
    for (i=0;charsets[i];i++)
	if (!strcasecmp(charsets[i],name)) return (rcc_autocharset_id)i;

    return (rcc_autocharset_id)-1;
}

int rccConfigInit(rcc_language_config config, rcc_context ctx) {
    unsigned int i;
    rcc_charset_id *charsets;
    rcc_charset_id *dcharsets;
    rcc_iconv *iconv_to;

    if ((!ctx)||(!config)) return -1;
    
    charsets = (rcc_charset_id*)malloc((ctx->max_classes)*sizeof(rcc_charset_id));
    dcharsets = (rcc_charset_id*)malloc((ctx->max_classes)*sizeof(rcc_charset_id));
    iconv_to = (rcc_iconv*)malloc((ctx->max_classes)*sizeof(rcc_iconv));
    if ((!charsets)||(!dcharsets)||(!iconv_to)) {
	if (dcharsets) free(dcharsets);
	if (charsets) free(charsets);
	if (iconv_to) free(iconv_to);
	return -1;
    }
    
    for (i=0;i<ctx->max_classes;i++) {
	dcharsets[i] = 0;
    	charsets[i] = 0;
	iconv_to[i] = NULL;
    }

    config->fsiconv = NULL;
    config->trans = NULL;
    config->entrans = NULL;

    config->ctx = ctx;
    config->language = NULL;
    config->charset = charsets;
    config->engine = -1;
    config->default_charset = dcharsets;
    config->configured = 0;
    config->speller = NULL;

    config->iconv_to = iconv_to;
    config->configure = 1;

    return 0;
}

void rccConfigFreeIConv(rcc_language_config config) {
    unsigned int i;

    if ((!config)||(!config->charset)) return;

    if (config->fsiconv) {
	rccIConvClose(config->fsiconv);
	config->fsiconv = NULL;
    }

    for (i=0;i<config->ctx->n_classes;i++) {
	if (config->iconv_to[i]) {
	    rccIConvClose(config->iconv_to[i]);
	    config->iconv_to[i] = NULL;
	}
    }
}

void rccConfigClear(rcc_language_config config) {
    if ((config)&&(config->charset)) {
	rccConfigFreeIConv(config);
	if (config->trans) {
	    rccTranslateClose(config->trans);
	    config->trans = NULL;
	}
	if (config->entrans) {
	    rccTranslateClose(config->entrans);
	    config->entrans = NULL;
	}
	if (config->iconv_to) {
	    free(config->iconv_to);
	    config->iconv_to = NULL;
	}
	if (config->charset) {
	    free(config->charset);
	    config->charset = NULL;
        }
	if (config->default_charset) {
	    free(config->default_charset);
	    config->default_charset = NULL;
	}
	if (config->speller) {
	    rccSpellerFree(config->speller);
	    config->speller = NULL;
	}
    }
}

static rcc_language_config rccGetConfigPointer(rcc_context ctx, rcc_language_id language_id, rcc_language_id *r_language_id) {

    language_id = rccGetRealLanguage(ctx, language_id);
    if (!strcasecmp(ctx->languages[language_id]->sn, rcc_disabled_language_sn)) return NULL;
    if (r_language_id) *r_language_id = language_id;

    return ctx->configs + language_id;
}

rcc_language_config rccCheckConfig(rcc_context ctx, rcc_language_id language_id) {
    rcc_language_config config;

    config = rccGetConfigPointer(ctx, language_id, NULL);
    if ((config)&&(!config->charset)) return NULL;

    return config;
}


rcc_language_config rccGetUsableConfig(rcc_context ctx, rcc_language_id language_id) {
    rcc_language_config config;

    config = rccGetConfigPointer(ctx, language_id, &language_id);
    if (config) {
	if (!rccCheckLanguageUsability(ctx, language_id)) return NULL;
	if ((!config->charset)&&(rccConfigInit(config, ctx))) return NULL;
	config->language = ctx->languages[language_id];
    }

    return config;
}

rcc_language_config rccGetConfig(rcc_context ctx, rcc_language_id language_id) {
    rcc_language_config config;

    config = rccGetConfigPointer(ctx, language_id, &language_id);
    if (config) {
	if ((!config->charset)&&(rccConfigInit(config, ctx))) return NULL;
	config->language = ctx->languages[language_id];
    }

    return config;
}

rcc_language_config rccGetConfigByName(rcc_context ctx, const char *name) {
    rcc_language_id language_id;
    
    language_id = rccGetLanguageByName(ctx, name);
    if (language_id == (rcc_language_id)-1) return NULL;
    
    return rccGetConfig(ctx, language_id);
}

rcc_language_config rccGetCurrentConfig(rcc_context ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetCurrentLanguage(ctx);
    if (language_id == (rcc_language_id)-1) return NULL;
    
    return rccGetConfig(ctx, language_id);
}

rcc_speller rccConfigGetSpeller(rcc_language_config config) {
    if (!config) return NULL;
    
    if (config->speller) return config->speller;

    config->speller = rccSpellerCreate(config->language->sn);
    return config->speller;
}

rcc_engine_id rccConfigGetSelectedEngine(rcc_language_config config) {
    if (!config) return (rcc_engine_id)-1;
    
    return config->engine;
}

const char *rccConfigGetSelectedEngineName(rcc_language_config config) {
    rcc_engine_id engine_id;
    
    engine_id = rccConfigGetSelectedEngine(config);
    if (engine_id == (rcc_engine_id)-1) return rcc_engine_nonconfigured;
    if (!config->language) return NULL;
    
    return rccConfigGetEngineName(config, engine_id);
}

rcc_engine_id rccConfigGetCurrentEngine(rcc_language_config config) {
    rcc_engine **enginelist;
    rcc_engine_id engine_id;
    
    if (!config) return (rcc_engine_id)-1;
    
    engine_id = rccConfigGetSelectedEngine(config);
    if (engine_id != (rcc_engine_id)-1) return engine_id;

    if (!config->language) return (rcc_engine_id)-1;
    else enginelist = config->language->engines;

    if (enginelist[0]) {
	if (enginelist[1]) return (rcc_engine_id)1;
	return (rcc_engine_id)0;
    }
    return (rcc_engine_id)-1;
}

const char *rccConfigGetCurrentEngineName(rcc_language_config config) {
    rcc_engine_id engine_id;
    
    engine_id = rccConfigGetCurrentEngine(config);
    if ((engine_id == (rcc_engine_id)-1)||(!config->language)) return NULL;
    
    return rccConfigGetEngineName(config, engine_id);
}

rcc_charset_id rccConfigGetSelectedCharset(rcc_language_config config, rcc_class_id class_id) {
    if ((!config)||(!config->ctx)||(class_id<0)||(class_id>=config->ctx->n_classes)) return (rcc_charset_id)-1;
    
    return config->charset[class_id];
}

const char *rccConfigGetSelectedCharsetName(rcc_language_config config, rcc_class_id class_id) {
    rcc_charset_id charset_id;
    
    charset_id = rccConfigGetSelectedCharset(config, class_id);
    if ((charset_id == (rcc_charset_id)-1)||(!config->language)) return NULL;
    
    return rccConfigGetCharsetName(config, charset_id);
}

rcc_charset_id rccConfigGetCurrentCharset(rcc_language_config config, rcc_class_id class_id) {
    unsigned int i;
    rcc_charset_id charset_id;

    rcc_class_default_charset *defcharset;
    const char *lang;

    rcc_language *language;
    rcc_class_ptr *classes;
    rcc_charset *charsets;

    rcc_class *cl;

    const char *defvalue;

    if ((!config)||(!config->ctx)||(class_id<0)||(class_id>=config->ctx->n_classes)) return -1;
    
    charset_id = config->charset[class_id];
    if (charset_id) return charset_id;
    
    if (!config->language) return (rcc_charset_id)-1;
    else language = config->language;

    classes = config->ctx->classes;
        
    cl = classes[class_id];
    defvalue = cl->defvalue;
    if (defvalue) {
	for (i=0;classes[i];i++) {
	    if (!strcmp(classes[i]->name, defvalue)) 
		return rccConfigGetCurrentCharset(config, i); 
	}
    } else defvalue = config->ctx->locale_variable;
    
    if (config->default_charset[class_id]) return config->default_charset[class_id];
    
    charset_id = rccConfigGetLocaleCharset(config, defvalue);
    if ((charset_id != 0)&&(charset_id != (rcc_charset_id)-1)) {
	config->default_charset[class_id] = charset_id;
	return charset_id;
    }
    
    if (cl->defvalue) {
	charset_id = rccConfigGetCharsetByName(config, defvalue);
	if ((charset_id != 0)&&(charset_id != (rcc_charset_id)-1)) {
	    config->default_charset[class_id] = charset_id;
	    return charset_id;
	}
    }
    
    defcharset = cl->defcharset;
    if (defcharset) {
	    lang = config->language->sn;
	    
	    for (i = 0; cl->defcharset[i].lang; i++) {
		if (!strcasecmp(lang, defcharset[i].lang)) {
		    charset_id = rccConfigGetCharsetByName(config, defcharset[i].charset);
		    if ((charset_id != 0)&&(charset_id != (rcc_charset_id)-1)) {
			config->default_charset[class_id] = charset_id;
			return charset_id;
		    } else break;
		}
	    }
    }	

    charset_id = rccConfigGetLocaleUnicodeCharset(config, defvalue);
    if ((charset_id != 0)&&(charset_id != (rcc_charset_id)-1)) {
	config->default_charset[class_id] = charset_id;
	return charset_id;
    }

    charsets=language->charsets;
    if ((charsets[0])&&(charsets[1])) {
	config->default_charset[class_id]=(rcc_charset_id)1;
	return (rcc_charset_id)1;
    }
    return (rcc_charset_id)-1;
}

const char *rccConfigGetCurrentCharsetName(rcc_language_config config, rcc_class_id class_id) {
    rcc_charset_id charset_id;
    
    charset_id = rccConfigGetCurrentCharset(config, class_id);
    if ((charset_id == (rcc_charset_id)-1)||(!config->language)) return NULL;
    
    return rccConfigGetCharsetName(config, charset_id);
}


int rccConfigSetEngine(rcc_language_config config, rcc_engine_id engine_id) {
    unsigned int i;
    
    if ((!config)||(!config->language)) return -1;

    if (engine_id != (rcc_engine_id)-1) {
	for (i=0;config->language->engines[i];i++);
	if (engine_id >= i) return -1;
    }
    
    if (config->engine != engine_id) {
	if (config->ctx->current_config == config) config->ctx->configure = 1;
	config->configure = 1;
	config->engine = engine_id;
    }

    config->configured = 1;

    return 0;
}

int rccConfigSetEngineByName(rcc_language_config config, const char *name) {
    rcc_engine_id engine_id;

    if (!config) return -1;
    
    if ((!name)||(!strcasecmp(name,rcc_engine_nonconfigured)))
	return rccConfigSetEngine(config, (rcc_engine_id)-1);
        
    engine_id = rccConfigGetEngineByName(config, name);
    if (engine_id == (rcc_engine_id)-1) return -1;
    
    return rccConfigSetEngine(config, engine_id);
}

int rccConfigSetCharset(rcc_language_config config, rcc_class_id class_id, rcc_charset_id charset_id) {
    unsigned int i;
    
    if ((!config)||(!config->language)||(class_id < 0)||(class_id >= config->ctx->n_classes)) return -1;


    for (i=0;config->language->charsets[i];i++);
    if (charset_id >= i) return -1;
    
    if (config->charset[class_id] != charset_id) {
	if (config->ctx->classes[class_id]->flags&RCC_CLASS_FLAG_CONST) return -1;

	if (config->ctx->current_config == config) config->ctx->configure = 1;
	config->configure = 1;
	config->charset[class_id] = charset_id;
    }

    config->configured = 1;

    return 0;
}

int rccConfigSetCharsetByName(rcc_language_config config, rcc_class_id class_id, const char *name) {
    rcc_charset_id charset_id;
    
    charset_id = rccConfigGetCharsetByName(config, name);
    if (charset_id == (rcc_charset_id)-1) return -1;
    
    return rccConfigSetCharset(config, class_id, charset_id);
}

rcc_charset_id rccConfigGetLocaleCharset(rcc_language_config config, const char *locale_variable) {
    const char *lv;
    rcc_language_id language_id;
    char stmp[RCC_MAX_CHARSET_CHARS+1];
    
    if ((!config)||(!config->language)) return (rcc_charset_id)-1;

    lv = locale_variable?locale_variable:config->ctx->locale_variable;

    language_id = rccGetLanguageByName(config->ctx, config->language->sn);
    if (language_id != (rcc_language_id)-1) {
	if (!rccLocaleGetLanguage(stmp, lv, RCC_MAX_CHARSET_CHARS)) {
	    if (!strcmp(config->language->sn, stmp)) {
		if (!rccLocaleGetCharset(stmp, lv, RCC_MAX_CHARSET_CHARS))
		    return rccConfigGetCharsetByName(config, stmp);
	    }
	}
    } 

    return (rcc_charset_id)-1;
}

rcc_charset_id rccConfigGetLocaleUnicodeCharset(rcc_language_config config, const char *locale_variable) {
    char stmp[RCC_MAX_CHARSET_CHARS+1];

    if ((!config)||(!config->language)) return (rcc_charset_id)-1;

    if (!rccLocaleGetCharset(stmp, locale_variable?locale_variable:config->ctx->locale_variable, RCC_MAX_CHARSET_CHARS)) {
	if (rccIsUTF8(stmp)) return rccConfigGetCharsetByName(config, stmp);
    } 
	
    return (rcc_charset_id)-1;
}

int rccConfigConfigure(rcc_language_config config) {
    rcc_context ctx;    
    const char *charset;
    unsigned int i;

    if (!config) return -1;
    if (!config->configure) return 0;

    ctx = config->ctx;
    if (!ctx) return -1;

    rccConfigFreeIConv(config);
    for (i=0;i<ctx->n_classes;i++) {
	charset = rccConfigGetCurrentCharsetName(config, (rcc_class_id)i);
	if ((!charset)||(rccIsUTF8(charset))) continue;
	config->iconv_to[i] = rccIConvOpen(charset, "UTF-8");
    }

    config->configure = 0;

    return 0;
}


rcc_string rccConfigSizedFrom(rcc_language_config config, rcc_class_id class_id, const char *buf, size_t len) {
    rcc_context ctx;
    rcc_string result;
    rcc_option_value usedb4;
    rcc_autocharset_id charset_id;
    const char *charset;


    if (!config) return NULL;
    ctx = config->ctx;

    if (rccStringSizedCheck(buf, len)) return NULL;
    
    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);

    if (usedb4&RCC_OPTION_LEARNING_FLAG_USE) {
	result = rccDb4GetKey(ctx->db4ctx, buf, len);
	if (result) {
	     if (rccStringFixID(result, ctx)) free(result);
	     else return result;
	}
    }

    charset_id = rccConfigDetectCharset(config, class_id, buf, len);
    if (charset_id != (rcc_autocharset_id)-1)
	charset = rccConfigGetAutoCharsetName(config, charset_id);
    else
	charset = rccConfigGetCurrentCharsetName(config, class_id);
    
    if (charset) {
	result = rccSizedFromCharset(ctx, charset, buf, len);
	if (result) rccStringChangeID(result, rccGetLanguageByName(ctx, config->language->sn));
	return result;
    }
    
    return NULL;
}

char *rccConfigSizedTo(rcc_language_config config, rcc_class_id class_id, rcc_const_string buf, size_t *rlen) {
    rcc_context ctx;
    const char *charset;

    if (!config) return NULL;
    ctx = config->ctx;

    charset = rccConfigGetCurrentCharsetName(config, class_id);

    if (charset)
	return rccSizedToCharset(ctx, charset, buf, rlen);
    
    return NULL;
}


char *rccConfigSizedRecode(rcc_language_config config, rcc_class_id from, rcc_class_id to, const char *buf, size_t len, size_t *rlen) {
    rcc_context ctx;
    rcc_string result;
    rcc_option_value usedb4;
    rcc_autocharset_id charset_id;
    rcc_string stmp;
    const char *tocharset, *fromcharset;


    if (!config) return NULL;
    ctx = config->ctx;

    if (rccStringSizedCheck(buf, len)) return NULL;
    
    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);

    if (usedb4&RCC_OPTION_LEARNING_FLAG_USE) {
	stmp = rccDb4GetKey(ctx->db4ctx, buf, len);
	if (stmp) {
	     if (rccStringFixID(stmp, ctx)) free(stmp);
	     else {
		result = rccConfigSizedTo(config, to, stmp, rlen);
		free(stmp);
		return result;
	    }
	}
    }

    charset_id = rccConfigDetectCharset(config, from, buf, len);
    if (charset_id != (rcc_autocharset_id)-1)
	fromcharset = rccConfigGetAutoCharsetName(config, charset_id);
    else
	fromcharset = rccConfigGetCurrentCharsetName(config, from);
    
    tocharset = rccConfigGetCurrentCharsetName(config, to);
    
    if ((fromcharset)&&(tocharset))
	return rccSizedRecodeCharsets(ctx, fromcharset, tocharset, buf, len, rlen);

    return NULL;

}


char *rccConfigSizedRecodeToCharset(rcc_language_config config, rcc_class_id class_id, const char *charset, rcc_const_string buf, size_t len, size_t *rlen) {
    rcc_context ctx;
    rcc_string result;
    rcc_option_value usedb4;
    rcc_autocharset_id charset_id;
    rcc_string stmp;
    const char *ocharset;


    if (!config) return NULL;
    ctx = config->ctx;

    if (rccStringSizedCheck(buf, len)) return NULL;
    
    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);

    if (usedb4&RCC_OPTION_LEARNING_FLAG_USE) {
	stmp = rccDb4GetKey(ctx->db4ctx, buf, len);
	if (stmp) {
	     if (rccStringFixID(stmp, ctx)) free(stmp);
	     else {
		result = rccSizedToCharset(ctx, charset, stmp, rlen);
		free(stmp);
	        return result;
	    }
	}
    }

    charset_id = rccConfigDetectCharset(config, class_id, buf, len);
    if (charset_id != (rcc_autocharset_id)-1)
	ocharset = rccConfigGetAutoCharsetName(config, charset_id);
    else
	ocharset = rccConfigGetCurrentCharsetName(config, class_id);
    
    if (ocharset)
	return rccSizedRecodeCharsets(ctx, ocharset, charset, buf, len, rlen);

    return NULL;
}

char *rccConfigSizedRecodeFromCharset(rcc_language_config config, rcc_class_id class_id, const char *charset, const char *buf, size_t len, size_t *rlen) {
    rcc_context ctx;
    const char *ocharset;

    if (!config) return NULL;
    ctx = config->ctx;

    ocharset = rccConfigGetCurrentCharsetName(config, class_id);

    if (ocharset)
	return rccSizedRecodeCharsets(ctx, charset, ocharset, buf, len, rlen);

    return NULL;
}


/*
    rcc_option_value options[RCC_MAX_OPTIONS];

int rccConfigInit(rcc_language_config config, rcc_context ctx) {
    for (i=0;i<RCC_MAX_OPTIONS;i++)
	config->options[i] = 0;    
}

rcc_option_value rccConfigGetOption(rcc_language_config config, rcc_option option) {
    if ((!config)||(option<0)||(option>=RCC_MAX_OPTIONS)) return -1;
    
    return config->options[option];
}

int rccConfigSetOption(rcc_language_config config, rcc_option option, rcc_option_value value) {
    if ((!config)||(option>=RCC_MAX_OPTIONS)) return -1;
    if (config->options[option] != value) {
	if (config->ctx->current_config == config) config->ctx->configure = 1;
	config->options[option]=value;
    }
    
    return 0;
}

rcc_option_value rccConfigGetOption(rcc_language_config config, rcc_option option);
int rccConfigSetOption(rcc_language_config config, rcc_option option, rcc_option_value value);
#define rccGetOption(ctx, option) rccConfigGetOption(ctx->current_config, option)
#define rccSetOption(ctx,option,value) rccConfigSetOption(ctx->current_config, option, value)
*/
