#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "rcciconv.h"
#include "fs.h"
#include "lng.h"
#include "rccstring.h"
#include "rccconfig.h"
#include "rccdb4.h"




static rcc_charset_id rccIConvAuto(rcc_context ctx, rcc_class_id class_id, const char *buf, int len) {
    rcc_class_type class_type;
    rcc_engine_ptr engine;
    
    if (!buf) return (rcc_charset_id)-1;

    class_type = rccGetClassType(ctx, class_id);
    if ((class_type == RCC_CLASS_STANDARD)||((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_TITLES)))) {
	engine = rccGetCurrentEnginePointer(ctx);
	if ((!engine)||(!engine->func)) return (rcc_charset_id)-1;
	return engine->func(&ctx->engine_ctx, buf, len);
    }
    
    return (rcc_charset_id)-1;
}

rcc_string rccFrom(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len, size_t *rlen) {
    int err;
    size_t ret;
    rcc_language_id language_id;
    rcc_charset_id charset_id;
    rcc_iconv icnv = NULL;
    rcc_string result;
    rcc_option_value usedb4;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((class_id<0)||(class_id>=ctx->n_classes)||(!buf)) return NULL;

	// Checking if rcc_string passed
    ret = rccStringSizedCheck(buf, len);
    if (ret) return NULL;
    
    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);

    if (usedb4&RCC_OPTION_LEARNING_FLAG_USE) {
	result = rccDb4GetKey(ctx->db4ctx, buf, len);
	if (result) {
	     if (rccStringFixID(result, ctx)) free(result);
	     else return result;
	}
    }

    err = rccConfigure(ctx);
    if (err) return NULL;
    
    language_id = rccGetCurrentLanguage(ctx);

    charset_id = rccIConvAuto(ctx, class_id, buf, len);
    if (charset_id != (rcc_charset_id)-1) icnv = ctx->iconv_auto[charset_id];
    else icnv = ctx->iconv_from[class_id];

    if (icnv) {
	ret = rccIConv(ctx, icnv, buf, len);
	if (ret == (size_t)-1) return NULL;
	result = rccCreateString(language_id, ctx->tmpbuffer, ret, rlen);
    } else {
	result = rccCreateString(language_id, buf, len, rlen);
    }

    if ((result)&&(usedb4&RCC_OPTION_LEARNING_FLAG_LEARN)) {
	if (!rccStringSetLang(result, ctx->languages[language_id]->sn)) {
	    rccDb4SetKey(ctx->db4ctx, buf, len, result);
	}
    }
    
    return result;
}

char *rccTo(rcc_context ctx, rcc_class_id class_id, const rcc_string buf, size_t len, size_t *rlen) {
    int err;
    size_t newlen;
    char *result;
    char *prefix, *name;
    const char *utfstring;
    rcc_language_config config;
    rcc_language_id language_id;
    rcc_charset_id charset_id;
    rcc_class_type class_type;
    rcc_iconv icnv;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((class_id<0)||(class_id>=ctx->n_classes)||(!buf)) return NULL;

    newlen = rccStringSizedCheck((const char*)buf, len);
    if (!newlen) return NULL;

    language_id = rccStringGetLanguage(buf);
    utfstring = rccStringGetString(buf);
    if ((!language_id)||(!utfstring)) return NULL;

    config = rccGetConfig(ctx, language_id);
    if (!config) return NULL;

    err = rccConfigConfigure(config);
    if (err) return NULL;

    class_type = rccGetClassType(ctx, class_id);
    if ((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_NAMES))) {
	    name = (char*)utfstring;
	    prefix = NULL;
	    
	    err = rccFS0(config, NULL, buf, &prefix, &name);
	    if (err>=0) {
		result = rccFS3(config, class_id, prefix, name);
		if (!err) {
		    if (prefix) free(prefix);
		    free(name);
		}
		if ((rlen)&&(result)) *rlen = strlen(result);
		return result;
	    }
    }

    icnv =  config->iconv_to[class_id];
    if (icnv) {
	newlen = rccIConv(ctx, icnv, rccStringGetString(buf), len?newlen:0);
	if (newlen == (size_t)-1) return NULL;

	result = rccCreateResult(ctx, newlen, rlen);
    } else {
	result = rccStringExtractString(buf);
	if (rlen) *rlen = newlen;
    }    
    
    return result;
}

char *rccRecode(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *buf, size_t len, size_t *rlen) {
    size_t nlen;
    rcc_string stmp;
    char *result;
    const char *from_charset, *to_charset;
    rcc_charset_id from_charset_id, to_charset_id;
    rcc_class_type class_type;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((from<0)||(from>=ctx->n_classes)||(to<0)||(to>=ctx->n_classes)||(!buf)) return NULL;

    class_type = rccGetClassType(ctx, to);
    if ((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_NAMES))) goto recoding;
    if (rccGetOption(ctx, RCC_OPTION_LEARNING_MODE)&RCC_OPTION_LEARNING_FLAG_LEARN) goto recoding;
    
    from_charset_id = rccIConvAuto(ctx, from, buf, len);
    if (from_charset_id != (rcc_charset_id)-1) {
	from_charset = rccGetAutoCharsetName(ctx, from_charset_id);
	to_charset = rccGetCurrentCharsetName(ctx, to);
	if ((from_charset)&&(to_charset)&&(!strcasecmp(from_charset, to_charset))) return NULL;
    } else {
	from_charset_id = rccGetCurrentCharset(ctx, from);
	to_charset_id = rccGetCurrentCharset(ctx, to);
	if (from_charset_id == to_charset_id) return NULL;
    }

recoding:    
    stmp = rccFrom(ctx, from, buf, len, &nlen);
    if (stmp) {
	result = rccTo(ctx, to, stmp, nlen, rlen);
	free(stmp);
	return result;
    } 
    
    return NULL;
}

char *rccFS(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *fspath, const char *path, const char *filename) {
    int err;
    rcc_language_config config;
    char *prefix = (char*)path, *name = (char*)filename; /*DS*/
    rcc_string string;

    char *stmp;
    char *result = NULL;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((from<0)||(from>=ctx->n_classes)||(to<0)||(to>=ctx->n_classes)||(!filename)) return NULL;
    
    config = rccGetCurrentConfig(ctx);
    if (!config) return NULL;
    
    err = rccFS1(config, fspath, &prefix, &name);
    if (err) {
	if (err < 0) return NULL;
	
	if (err&1) {
	    if (err&2) return NULL;
	    if (rccGetOption(ctx, RCC_OPTION_LEARNING_MODE)&RCC_OPTION_LEARNING_FLAG_LEARN) {
	        string = rccFrom(ctx, from, name, 0, NULL);
		if (string) free(string);
	    }
	    return name;
	}
    }

    string = rccFrom(ctx, from, name, 0, NULL);
    if (string) {
	config = rccGetConfig(ctx, rccStringGetLanguage(string));
	if (config) result = rccFS3(config, to, prefix, rccStringGetString(string));
	else result = NULL;
	free(string);
    } else result = NULL;
    

    if (!(err&2)) {
	if (prefix) free(prefix);
	free(name);
    }
    return result;
}
