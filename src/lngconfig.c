#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "rccconfig.h"
#include "rcclocale.h"

rcc_engine_ptr rccConfigGetEnginePointer(rcc_language_config config, rcc_engine_id engine_id) {
    unsigned int i;
    rcc_engine_ptr *engines;
    
    if ((!config)||(!config->language)||(engine_id<0)) return NULL;

    engines = config->language->engines;
    
    for (i=0;engines[i];i++);
    if (engine_id>=i) return NULL;
    
    return engines[engine_id];
}

const char *rccConfigGetEngineName(rcc_language_config config, rcc_engine_id engine_id) {
    rcc_engine_ptr engine;
    
    engine = rccConfigGetEnginePointer(config, engine_id);
    if (!engine) return NULL;
    return engine->title;
}

const char *rccConfigGetCharsetName(rcc_language_config config, rcc_charset_id charset_id) {
    unsigned int i;
    rcc_charset *charsets;
    
    if ((!config)||(!config->language)||(charset_id<0)) return NULL;

    charsets = config->language->charsets;
    
    for (i=0;charsets[i];i++);
    if (charset_id>=i) return NULL;
    
    return charsets[charset_id];
}

const char *rccConfigGetAutoCharsetName(rcc_language_config config, rcc_charset_id charset_id) {
    unsigned int i;
    rcc_charset *charsets;
    rcc_engine_ptr *engines;
    
    if ((!config)||(!config->language)||(charset_id<0)) return NULL;

    engines = config->language->engines;
    charsets = engines[config->engine]->charsets;

    for (i=0;charsets[i];i++);
    if (charset_id>=i) return NULL;
    
    return charsets[charset_id];
}


rcc_engine_id rccConfigGetEngineByName(rcc_language_config config, const char *name) {
    unsigned int i;
    rcc_engine **engines;

    if ((!config)||(!config->language)||(!name)) return -1;
    
    engines = config->language->engines;
    for (i=0;engines[i];i++)
	if (!strcmp(engines[i]->title,name)) return i;
    
    return -1;
}

rcc_charset_id rccConfigGetCharsetByName(rcc_language_config config, const char *name) {
    unsigned int i;
    rcc_charset *charsets;

    if ((!config)||(!config->language)||(!name)) return -1;
    
    charsets = config->language->charsets;
    for (i=0;charsets[i];i++)
	if (!strcmp(charsets[i],name)) return i;
    
    return 0;
}

rcc_charset_id rccConfigGetAutoCharsetByName(rcc_language_config config, const char *name) {
    unsigned int i;
    rcc_charset *charsets;
    rcc_engine_ptr *engines;

    if ((!config)||(!config->language)||(!name)) return -1;

    engines = config->language->engines;
    charsets = engines[config->engine]->charsets;
    
    for (i=0;charsets[i];i++)
	if (!strcmp(charsets[i],name)) return i;
    
    return -1;
}

int rccConfigInit(rcc_language_config config, rcc_context ctx) {
    unsigned int i;
    rcc_charset_id *charsets;

    if ((!ctx)||(!config)) return -1;
    
    charsets = (rcc_charset_id*)malloc((ctx->max_classes)*sizeof(rcc_charset_id));
    if (!charsets) return -1;
    
    for (i=0;i<ctx->max_classes;i++)
	charsets[i] = 0;

    config->ctx = ctx;
    config->language = NULL;
    config->charset = charsets;
    config->engine = -1;

    return 0;
}

int rccConfigFree(rcc_language_config config) {
    if (config->charset) {
	free(config->charset);
	config->charset = NULL;
    }
}

rcc_language_config rccCheckConfig(rcc_context ctx, rcc_language_id language_id) {
    rcc_language_id new_language_id;
    int err;
    
    new_language_id = rccGetRealLanguage(ctx, language_id);
    if ((language_id<=0)||(new_language_id != language_id)) return NULL;
    if (!ctx->configs[language_id].charset) return NULL;
    if (!strcasecmp(ctx->languages[language_id]->sn, "off")) return NULL;

    return ctx->configs + language_id;
}

rcc_language_config rccGetConfig(rcc_context ctx, rcc_language_id language_id) {
    int err;

    language_id = rccGetRealLanguage(ctx, language_id);
    if (language_id < 0) return NULL;
    if (!ctx->configs[language_id].charset) {
	if (rccConfigInit(ctx->configs+language_id, ctx)) return NULL;
    }    

    ctx->configs[language_id].language = ctx->languages[language_id];
    return ctx->configs + language_id;
}

rcc_language_config rccGetConfigByName(rcc_context ctx, const char *name) {
    rcc_language_id language_id;
    
    language_id = rccGetLanguageByName(ctx, name);
    if (language_id < 0) return NULL;
    
    return rccGetConfig(ctx, language_id);
}

rcc_language_config rccGetCurrentConfig(rcc_context ctx) {
    rcc_language_id language_id;
    
    language_id = rccGetCurrentLanguage(ctx);
    if (language_id < 0) return NULL;
    
    return rccGetConfig(ctx, language_id);
}

rcc_engine_id rccConfigGetSelectedEngine(rcc_language_config config) {
    if (!config) return -1;
    
    return config->engine;
}

const char *rccConfigGetSelectedEngineName(rcc_language_config config) {
    rcc_engine_id engine_id;
    
    engine_id = rccConfigGetSelectedEngine(config);
    if (engine_id == -1) return rcc_engine_nonconfigured;
    if ((engine_id < 0)||(!config->language)) return NULL;
    
    return rccConfigGetEngineName(config, engine_id);
}

rcc_engine_id rccConfigGetCurrentEngine(rcc_language_config config) {
    rcc_engine **enginelist;
    rcc_engine_id engine_id;
    
    engine_id = rccConfigGetSelectedEngine(config);
    if (engine_id>=0) return engine_id;

    if (!config->language) return -1;
    else enginelist = config->language->engines;

    if (enginelist[0]) {
	if (enginelist[1]) return 1;
	return 0;
    }
    return -1;
}

const char *rccConfigGetCurrentEngineName(rcc_language_config config) {
    rcc_engine_id engine_id;
    
    engine_id = rccConfigGetCurrentEngine(config);
    if ((engine_id < 0)||(!config->language)) return NULL;
    
    return rccConfigGetEngineName(config, engine_id);
}

rcc_charset_id rccConfigGetSelectedCharset(rcc_language_config config, rcc_class_id class_id) {
    if ((!config)||(!config->ctx)||(class_id<0)||(class_id>=config->ctx->n_classes)) return -1;
    
    return config->charset[class_id];
}

const char *rccConfigGetSelectedCharsetName(rcc_language_config config, rcc_class_id class_id) {
    rcc_charset_id charset_id;
    
    charset_id = rccConfigGetSelectedCharset(config, class_id);
    if ((charset_id < 0)||(!config->language)) return NULL;
    
    return rccConfigGetCharsetName(config, charset_id);
}

rcc_charset_id rccConfigGetCurrentCharset(rcc_language_config config, rcc_class_id class_id) {
    int err;
    unsigned int i;
    rcc_language_id language_id;
    rcc_charset_id charset_id;

    rcc_language *language;
    rcc_class_ptr *classes;
    rcc_charset *charsets;

    rcc_class *cl;

    char stmp[RCC_MAX_CHARSET_CHARS + 1];
    const char *defvalue;
    
    if ((!config)||(!config->ctx)||(class_id<0)||(class_id>=config->ctx->n_classes)) return -1;
    
    charset_id = config->charset[class_id];
    if (charset_id) return charset_id;
    
    if (!config->language) return -1;
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

    charset_id = rccConfigGetLocaleCharset(config, defvalue);
    if (charset_id > 0) return charset_id;
	
    charsets=config->ctx->languages[language_id]->charsets;
    if ((charsets[0])&&(charsets[1])) return 1;
    return -1;
}

const char *rccConfigGetCurrentCharsetName(rcc_language_config config, rcc_class_id class_id) {
    rcc_charset_id charset_id;
    
    charset_id = rccConfigGetCurrentCharset(config, class_id);
    if ((charset_id < 0)||(!config->language)) return NULL;
    
    return rccConfigGetCharsetName(config, charset_id);
}


int rccConfigSetEngine(rcc_language_config config, rcc_engine_id engine_id) {
    unsigned int i;
    
    if ((!config)||(!config->language)||(engine_id < -1)) return -1;

    if (engine_id != (rcc_engine_id)-1) {
	for (i=0;config->language->engines[i];i++);
	if (engine_id >= i) return -1;
    }
    
    if (config->engine != engine_id) {
	if (config->ctx->current_config == config) config->ctx->configure = 1;
	config->engine = engine_id;
    }
    return 0;
}

int rccConfigSetEngineByName(rcc_language_config config, const char *name) {
    rcc_engine_id engine_id;

    if (!config) return -1;

    if ((!name)||(!strcasecmp(name,rcc_engine_nonconfigured)))
	return rccConfigSetEngine(config, (rcc_engine_id)-1);
        
    engine_id = rccConfigGetEngineByName(config, name);
    if (engine_id < 0) return -1;
    
    return rccConfigSetEngine(config, engine_id);
}

int rccConfigSetCharset(rcc_language_config config, rcc_class_id class_id, rcc_charset_id charset_id) {
    unsigned int i;
    
    if ((!config)||(!config->language)||(class_id < 0)||(class_id >= config->ctx->n_classes)||(charset_id<0)) return -1;

    for (i=0;config->language->charsets[i];i++);
    if (charset_id >= i) return -1;
    
    if (config->charset[class_id] != charset_id) {
	if (config->ctx->current_config == config) config->ctx->configure = 1;
	config->charset[class_id] = charset_id;
    }

    return 0;
}

int rccConfigSetCharsetByName(rcc_language_config config, rcc_class_id class_id, const char *name) {
    rcc_charset_id charset_id;
    
    charset_id = rccConfigGetCharsetByName(config, name);
    if (charset_id < 0) return -1;
    
    return rccConfigSetCharset(config, class_id, charset_id);
}

rcc_charset_id rccConfigGetLocaleCharset(rcc_language_config config, const char *locale_variable) {
    int err;    
    rcc_charset *charsets;
    rcc_language_id language_id;
    char stmp[RCC_MAX_CHARSET_CHARS+1];
    
    if ((!config)||(!config->language)) return -1;

    language_id = rccGetCurrentLanguage(config->ctx);
    if (language_id) err = rccLocaleGetLanguage(stmp, locale_variable?locale_variable:config->ctx->locale_variable, RCC_MAX_CHARSET_CHARS);

    if ((language_id == 0)||((!err)&&(!strcmp(rccGetCurrentLanguageName(config->ctx), stmp))))
	err = rccLocaleGetCharset(stmp, locale_variable?locale_variable:config->ctx->locale_variable, RCC_MAX_CHARSET_CHARS);
    else 
	err = 1;
	
    if (err) {
	charsets=config->language->charsets;
	if ((charsets[0])&&(charsets[1])) return 1;
	return -1;
    }

    return rccConfigGetCharsetByName(config, stmp);
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
