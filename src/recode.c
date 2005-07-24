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




static rcc_autocharset_id rccIConvAuto(rcc_context ctx, rcc_class_id class_id, const char *buf, int len) {
    rcc_class_type class_type;
    rcc_engine_ptr engine;
    
    if (!buf) return (rcc_autocharset_id)-1;
    
    class_type = rccGetClassType(ctx, class_id);
    if ((class_type != RCC_CLASS_FS)||((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_TITLES)))) {
	engine = rccGetCurrentEnginePointer(ctx);
	if ((!engine)||(!engine->func)) return (rcc_autocharset_id)-1;
	return engine->func(&ctx->engine_ctx, buf, len);
    }
    
    return (rcc_autocharset_id)-1;
}

rcc_string rccSizedFrom(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    int err;
    size_t ret;
    rcc_language_id language_id;
    rcc_autocharset_id charset_id;
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
    
    language_id = rccGetCurrentLanguage(ctx);
    if (language_id == (rcc_language_id)-1) return NULL;
    if (!strcasecmp(ctx->languages[language_id]->sn, "off")) return NULL;

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
    
    charset_id = rccIConvAuto(ctx, class_id, buf, len);
    if (charset_id != (rcc_autocharset_id)-1) icnv = ctx->iconv_auto[charset_id];
    else icnv = ctx->iconv_from[class_id];

    if (icnv) {
	ret = rccIConv(ctx, icnv, buf, len);
	if (ret == (size_t)-1) return NULL;
	result = rccCreateString(language_id, ctx->tmpbuffer, ret);
    } else {
	result = rccCreateString(language_id, buf, len);
    }

    if ((result)&&(usedb4&RCC_OPTION_LEARNING_FLAG_LEARN)) {
	if (!rccStringSetLang(result, ctx->languages[language_id]->sn)) {
	    rccDb4SetKey(ctx->db4ctx, buf, len, result);
	}
    }
    
    return result;
}

char *rccSizedTo(rcc_context ctx, rcc_class_id class_id, rcc_const_string buf, size_t *rlen) {
    int err;
    size_t newlen;
    char *result;
    char *prefix, *name;
    const char *utfstring;
    rcc_language_config config;
    rcc_language_id language_id;
    rcc_class_type class_type;
    rcc_iconv icnv;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((class_id<0)||(class_id>=ctx->n_classes)||(!buf)) return NULL;

    newlen = rccStringCheck((const char*)buf);
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
	    if (rccIsASCII(utfstring)) {
		result = rccStringExtractString(buf);
		if ((result)&&(rlen)) *rlen = strlen(result);
		return result;
	    }

	    name = (char*)utfstring;
	    prefix = NULL;
	    
	    err = rccFS0(config, NULL, buf, &prefix, &name);
	    if (err>=0) {
		result = rccFS3(config, class_id, prefix, name);
		if (!err) {
		    if (prefix) free(prefix);
		    free(name);
		}
		if (rlen) *rlen = strlen(result);
		return result;
	    }
    }

    icnv =  config->iconv_to[class_id];
    if (icnv) {
	newlen = rccIConv(ctx, icnv, rccStringGetString((const char*)buf), newlen);
	if (newlen == (size_t)-1) return NULL;

	result = rccCreateResult(ctx, newlen);
	if (rlen) *rlen = newlen;
    } else {
	result = rccStringExtractString(buf);
	if (rlen) *rlen = newlen;
    }    
    
    return result;
}

char *rccSizedRecode(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *buf, size_t len, size_t *rlen) {
    int err;
    rcc_string stmp;
    char *result;
    const char *from_charset, *to_charset;
    rcc_charset_id from_charset_id, to_charset_id;
    rcc_class_type class_type;
    rcc_option_value usedb4;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((from<0)||(from>=ctx->n_classes)||(to<0)||(to>=ctx->n_classes)||(!buf)) return NULL;

    class_type = rccGetClassType(ctx, to);
    if ((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_NAMES))) goto recoding;
    if (rccGetOption(ctx, RCC_OPTION_LEARNING_MODE)&RCC_OPTION_LEARNING_FLAG_LEARN) goto recoding;

    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);
    if (usedb4&RCC_OPTION_LEARNING_FLAG_USE) {
	stmp = rccDb4GetKey(ctx->db4ctx, buf, len);
	if (stmp) {
	     if (rccStringFixID(stmp, ctx)) free(stmp);
	     else {
		result = rccSizedTo(ctx, to, stmp, rlen);
		free(stmp);
		return result;
	    }
	}
    }

    err = rccConfigure(ctx);
    if (err) return NULL;
    
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
    stmp = rccSizedFrom(ctx, from, buf, len);
    if (stmp) {
	result = rccSizedTo(ctx, to, stmp, rlen);
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
	    if (rccGetOption(ctx, RCC_OPTION_LEARNING_MODE)&RCC_OPTION_LEARNING_FLAG_LEARN) {
	        string = rccFrom(ctx, from, name);
		if (string) free(string);
	    }
	    if (err&2) return NULL;
	    return name;
	}
    }

    string = rccFrom(ctx, from, name);
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


rcc_string rccSizedFromCharset(rcc_context ctx, const char *charset, const char *buf, size_t len) {
    rcc_iconv icnv;
    rcc_language_config config;
    rcc_language_id language_id;
    size_t res;
    
    if ((!buf)||(!charset)) return NULL;
    
    language_id = rccGetCurrentLanguage(ctx);
    if ((language_id == (rcc_language_id)-1)||(language_id == 0)) return NULL;
    config = rccGetConfig(ctx, language_id);
    if (!config) return NULL;
    
    icnv = rccIConvOpen("UTF-8", charset);
    if (icnv) {
	res = rccIConv(ctx, icnv, buf, len);
	rccIConvClose(icnv);
	if (res == (size_t)-1) return NULL;
	return rccCreateString(language_id, ctx->tmpbuffer, res);
    } 
    return rccCreateString(language_id, buf, len);
}

char *rccSizedToCharset(rcc_context ctx, const char *charset, rcc_const_string buf, size_t *rlen) {
    rcc_iconv icnv;
    size_t res;
    
    if ((!buf)||(!charset)) return NULL;

    res = rccStringCheck(buf);
    if (!res) return NULL;
    
    icnv = rccIConvOpen(charset, "UTF-8");
    if (icnv) {
	res = rccIConv(ctx, icnv, rccStringGetString(buf), res);
	rccIConvClose(icnv);
	if (res == (size_t)-1) return NULL;
	
	if (rlen) *rlen = res;
	return rccCreateResult(ctx, res);
    } 

    if (rlen) *rlen = res;
    return rccStringExtractString(buf);
}

/* Convert from class_id to Charset */
char *rccSizedRecodeToCharset(rcc_context ctx, rcc_class_id class_id, const char *charset, rcc_const_string buf, size_t len, size_t *rlen) {
    size_t res;
    rcc_iconv icnv;
    const char *str;
    char *utf8, *extracted;

    if (!charset) return NULL;
    
    utf8 = rccSizedFrom(ctx, class_id, buf, len);
    if (!utf8) return utf8;
    
    str = rccStringGetString(utf8);

    icnv = rccIConvOpen(charset, "UTF-8");
    if (icnv) {
	res = rccIConv(ctx, icnv, str, 0);
	rccIConvClose(icnv);
	free(utf8);

	if (res == (size_t)-1) return NULL;
	if (rlen) *rlen = res;
	return rccCreateResult(ctx, res);
    }
    
    extracted = rccStringExtractString(utf8);
    free(utf8);

    if ((rlen)&&(extracted)) *rlen = strlen(extracted);
    return extracted;
}

/* Convert to class_id from Charset */
char *rccSizedRecodeFromCharset(rcc_context ctx, rcc_class_id class_id, const char *charset, const char *buf, size_t len, size_t *rlen) {
    size_t res;
    rcc_iconv icnv;
    rcc_string str;
    char *extracted;
    

    if (!charset) return NULL;
    
    icnv = rccIConvOpen("UTF-8", charset);
    if (icnv) {
	res = rccIConv(ctx, icnv, buf, len);
	rccIConvClose(icnv);

	if (res == (size_t)-1) return NULL;

	str = rccCreateString(rccGetCurrentLanguage(ctx), ctx->tmpbuffer, res);
    } else str = rccCreateString(rccGetCurrentLanguage(ctx), buf, len);

    if (!str) return NULL;

    extracted = rccSizedTo(ctx, class_id, str, rlen);
    free(str);

    return extracted;
}

char *rccSizedRecodeCharsets(rcc_context ctx, const char *from, const char *to, const char *buf, size_t len, size_t *rlen) {
    char *str;
    size_t res;
    rcc_iconv icnv;

    icnv = rccIConvOpen(to, from);
    if (!icnv) return NULL;

    res = rccIConv(ctx, icnv, buf, len);
    rccIConvClose(icnv);
    
    if (res == (size_t)-1) return NULL;
    
    

    str = (char*)malloc((res+1)*sizeof(char));
    if (!str) return NULL;
    memcpy(str, ctx->tmpbuffer, res);
    if (rlen) *rlen = res;

    return str;
}
