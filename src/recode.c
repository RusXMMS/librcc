#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <iconv.h>

#include "internal.h"
#include "fs.h"
#include "lng.h"
#include "rccstring.h"
#include "rccconfig.h"


static void rccIConvCopySymbol(char **in_buf, int *in_left, char **out_buf, int *out_left) {
    if ((out_left>0)&&(in_left>0)) {
	(**out_buf)=(**in_buf);
	(*out_buf)++;
	(*in_buf)++;
	(*in_left)--;
	(*out_left)--;
    }
}

static int rccIConvUTFBytes(unsigned char c) {
    int j;
    if (c<128) return 1;

    for (j=6;j>=0;j--)
	if ((c&bit(j))==0) break;
	    
    if ((j==0)||(j==6)) return 1;
    return 6-j;
}

static int rccIConv(rcc_context ctx, iconv_t icnv, const char *buf, int len) {
    char *in_buf, *out_buf, *res, err;
    int in_left, out_left, olen;
    int ub, utf_mode=0;
    int errors=0;
    
    if ((!buf)||(!ctx)||(icnv == (iconv_t)-1)) return -1;
    
    len = STRNLEN(buf,len);
    
    if (iconv(icnv, NULL, NULL, NULL, NULL) == -1) return -1;
    
loop_restart:
    errors = 0;
    in_buf = (char*)buf; /*DS*/
    in_left = len;
    out_buf = ctx->tmpbuffer;
    out_left = RCC_MAX_STRING_CHARS;

loop:
    err=iconv(icnv, &in_buf, &in_left, &out_buf, &out_left);
    if (err<0) {
        if (errno==E2BIG) {
    	    *(int*)(ctx->tmpbuffer+(RCC_MAX_STRING_CHARS-sizeof(int)))=0;
	} else if (errno==EILSEQ) {
	    if (errors++<RCC_MAX_ERRORS) {
		for (ub=utf_mode?rccIConvUTFBytes(*in_buf):1;ub>0;ub--)
		    rccIConvCopySymbol(&in_buf, &in_left, &out_buf, &out_left);
		if (in_left>0) goto loop;
	    } else if (!utf_mode) {
		utf_mode = 1;
		goto loop_restart;
	    } else {
	        return -1;
	    }
	} else {
	    return -1;
	}
    }
        
    return RCC_MAX_STRING_CHARS - out_left;
}


static rcc_charset_id rccIConvAuto(rcc_context ctx, rcc_class_id class_id, const char *buf, int len) {
    rcc_class_type class_type;
    rcc_engine_ptr engine;
    
    if ((!ctx)||(!buf)) return -1;

    class_type = rccGetClassType(ctx, class_id);
    if ((class_type == RCC_CLASS_STANDARD)||((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_AUTODETECT_FS_TITLES)))) {
	engine = rccGetEnginePointer(ctx, rccGetCurrentEngine(ctx));
	if ((!engine)||(!engine->func)||(!stricmp(engine->title, "off"))||(!strcmp(engine->title, "dissable"))) return -1;
	return engine->func(&ctx->engine_ctx, buf, len);
    }
    
    return -1;
}

rcc_string rccFrom(rcc_context ctx, rcc_class_id class_id, const char *buf, int len, int *rlen) {
    int err;
    rcc_language_id language_id;
    rcc_charset_id charset_id;
    iconv_t icnv = (iconv_t)-1;
    rcc_string result;

    if ((!ctx)||(class_id<0)||(class_id>=ctx->n_classes)||(!buf)) return NULL;
    
    err = rccConfigure(ctx);
    if (err) return NULL;

    language_id = rccGetCurrentLanguage(ctx);
    // DS: Learning. check database (language_id)

    charset_id = rccIConvAuto(ctx, class_id, buf, len);
    if (charset_id > 0) icnv = ctx->iconv_auto[charset_id];
    if (icnv == (iconv_t)-1) {
	icnv = ctx->iconv_from[class_id];
	if (icnv == (iconv_t)-1) return NULL;
    }
    
    if (icnv == (iconv_t)-2) {
	result = rccCreateString(language_id, buf, len, rlen);
    } else { 
	err = rccIConv(ctx, icnv, buf, len);
	if (err<=0) return NULL;
	result = rccCreateString(language_id, ctx->tmpbuffer, err, rlen);
    }
    
    // DS: Learning. write database
    
    return result;
}

char *rccTo(rcc_context ctx, rcc_class_id class_id, const rcc_string buf, int len, int *rlen) {
    int err;
    char *result;
    char *prefix, *name;
    rcc_language_id language_id;
    rcc_charset_id charset_id;
    rcc_class_type class_type;
    iconv_t icnv;
    
    if ((!ctx)||(class_id<0)||(class_id>=ctx->n_classes)||(!buf)) return NULL;

    language_id = rccCheckString(ctx, buf);
    if (!language_id) return NULL;

    err = rccConfigure(ctx);
    if (err) return NULL;

    icnv =  ctx->iconv_to[class_id];

    class_type = rccGetClassType(ctx, class_id);
    if ((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES))) {
	// DS: file_names (aut odetect fspath)
	    prefix = NULL; name = buf + sizeof(rcc_string_header);
	    err = rccFS0(NULL, buf, &prefix, &name);
	    if (!err) {
		result = rccFS3(ctx, language_id, class_id, prefix, name);
		if ((rlen)&&(result)) *rlen = strlen(result);
		return result;
	    }
    }
    
    if (icnv == (iconv_t)-1) return NULL;
    if (icnv == (iconv_t)-2) {
	result = rccStringExtract(buf, len, rlen);
    } else {
	err = rccIConv(ctx, icnv, buf + sizeof(rcc_string_header), len?len-sizeof(rcc_string_header):0);
	if (err<=0) return NULL;

	result = rccCreateResult(ctx, err, rlen);
    }
    
    return result;
}

char *rccRecode(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *buf, int len, int *rlen) {
    int nlen;
    rcc_string stmp;
    char *result;
    const char *from_charset, *to_charset;
    rcc_charset_id from_charset_id, to_charset_id;
    rcc_class_type class_type;

    if ((!ctx)||(from<0)||(from>=ctx->n_classes)||(to<0)||(to>=ctx->n_classes)||(!buf)) return NULL;

    class_type = rccGetClassType(ctx, to);
    if ((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES))) goto recoding;
    
    from_charset_id = rccIConvAuto(ctx, from, buf, len);
    if (from_charset_id>0) {
	from_charset = rccGetAutoCharsetName(ctx, from_charset_id);
	to_charset = rccGetCurrentCharsetName(ctx, to);
	if ((from_charset)&&(to_charset)&&(!stricmp(from_charset, to_charset))) return NULL;
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
    
    
    err = rccFS1(ctx, fspath, &prefix, &name);
    if (err) {
	if (err<0) return NULL;
	return name;
    }

    string = rccFrom(ctx, from, name, 0, NULL);
    if (string) {
	language_id = rccGetCurrentLanguage(ctx);
	result = rccFS3(ctx, language_id, to, prefix, string + sizeof(rcc_string_header));
	free(string);
    } 
    
    free(prefix);
    free(name);

    return result;
}
