#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "rcciconv.h"
#include "fs.h"
#include "lng.h"
#include "rccstring.h"
#include "rccconfig.h"




static rcc_charset_id rccIConvAuto(rcc_context ctx, rcc_class_id class_id, const char *buf, int len) {
    rcc_class_type class_type;
    rcc_engine_ptr engine;
    
    if (!buf) return (rcc_charset_id)-1;

    class_type = rccGetClassType(ctx, class_id);
    if ((class_type == RCC_CLASS_STANDARD)||((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_AUTODETECT_FS_TITLES)))) {
	engine = rccGetEnginePointer(ctx, rccGetCurrentEngine(ctx));
	if ((!engine)||(!engine->func)||(!strcasecmp(engine->title, "off"))||(!strcasecmp(engine->title, "dissable"))) return -1;
	return engine->func(&ctx->engine_ctx, buf, len);
    }
    
    return (rcc_charset_id)-1;
}

rcc_string rccFrom(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len, size_t *rlen) {
    int err;
    size_t ret;
    rcc_language_id language_id;
    rcc_charset_id charset_id;
    iconv_t icnv = (iconv_t)-1;
    rcc_string result;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((class_id<0)||(class_id>=ctx->n_classes)||(!buf)) return NULL;
    
    err = rccConfigure(ctx);
    if (err) return NULL;

	// Checking if rcc_string passed
    ret = rccStringSizedCheck(buf, len);
    if (ret) return NULL;

    language_id = rccGetCurrentLanguage(ctx);
    // DS: Learning. check database (language_id)

    charset_id = rccIConvAuto(ctx, class_id, buf, len);
    if (charset_id != (rcc_charset_id)-1) icnv = ctx->iconv_auto[charset_id];

    if (icnv == (iconv_t)-1) {
	icnv = ctx->iconv_from[class_id];
	if (icnv == (iconv_t)-1) return NULL;
    }
    
    if (icnv == (iconv_t)-2) {
	result = rccCreateString(language_id, buf, len, rlen);
    } else { 
	ret = rccIConv(ctx, icnv, buf, len);
	if (ret == (size_t)-1) return NULL;
	result = rccCreateString(language_id, ctx->tmpbuffer, ret, rlen);
    }

    // DS: Learning. write database
    
    return result;
}

char *rccTo(rcc_context ctx, rcc_class_id class_id, const rcc_string buf, size_t len, size_t *rlen) {
    int err;
    size_t newlen;
    char *result;
    char *prefix, *name;
    const char *utfstring;
    rcc_language_id language_id;
    rcc_charset_id charset_id;
    rcc_class_type class_type;
    iconv_t icnv;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((class_id<0)||(class_id>=ctx->n_classes)||(!buf)) return NULL;

    newlen = rccStringSizedCheck((const char*)buf, len);
    if (!newlen) return NULL;

    language_id = rccStringGetLanguage(buf);
    utfstring = rccStringGetString(buf);

    err = rccConfigure(ctx);
    if (err) return NULL;

    icnv =  ctx->iconv_to[class_id];

    class_type = rccGetClassType(ctx, class_id);
    if ((language_id)&&(class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES))) {
	    name = (char*)utfstring;
	    prefix = NULL;
	    
	    err = rccFS0(ctx, NULL, buf, &prefix, &name);
	    if (err>=0) {
		result = rccFS3(ctx, language_id, class_id, prefix, name);
		if (!err) {
		    if (prefix) free(prefix);
		    free(name);
		}
		if ((rlen)&&(result)) *rlen = strlen(result);
		return result;
	    }
    }
    
    if (icnv == (iconv_t)-1) return NULL;
    if (icnv == (iconv_t)-2) {
	result = rccStringExtractString(buf);
	if (rlen) *rlen = newlen;
    } else {
	newlen = rccIConv(ctx, icnv, rccStringGetString(buf), len?newlen:0);
	if (newlen == (size_t)-1) return NULL;

	result = rccCreateResult(ctx, newlen, rlen);
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
    if ((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES))) goto recoding;
    
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
    
    /*return rccTo(ctx, to, buf, len, rlen);*/
    return NULL;
}

char *rccFS(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *fspath, const char *path, const char *filename) {
    int err;
    rcc_language_id language_id;
    char *prefix = (char*)path, *name = (char*)filename; /*DS*/
    rcc_string string;

    char *stmp;
    char *result = NULL;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    if ((from<0)||(from>=ctx->n_classes)||(to<0)||(to>=ctx->n_classes)||(!filename)) return NULL;
    
    err = rccFS1(ctx, fspath, &prefix, &name);
    if (err) {
	if (err < 0) return NULL;
	
	if (err&1) {
	    if (err&2) return NULL;
	    return name;
	}
    }

    string = rccFrom(ctx, from, name, 0, NULL);
    if (string) {
	language_id = rccGetCurrentLanguage(ctx);
	result = rccFS3(ctx, language_id, to, prefix, rccStringGetString(string));
	free(string);
    } else result = NULL;
    
    if (!(err&2)) {
	if (prefix) free(prefix);
	free(name);
    }

    return result;
}
