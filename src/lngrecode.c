#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#include "internal.h"
#include "fs.h"

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

/* The supplied config have priority over language tag in the buf! */
char *rccConfigSizedTo(rcc_language_config config, rcc_class_id class_id, rcc_const_string buf, size_t *rlen) {
    char *result;
    const char *charset;

    if (!config) return NULL;

    if ((rccGetClassType(config->ctx, class_id) == RCC_CLASS_FS)&&(rccGetOption(config->ctx, RCC_OPTION_AUTODETECT_FS_NAMES))) {
	result = rccFS5(config->ctx, config, class_id, rccStringGetString(buf));
	if (result) {
	    if (rlen) *rlen = strlen(result);
	    return result;
	}
    }

    charset = rccConfigGetCurrentCharsetName(config, class_id);

    if (charset)
	return rccSizedToCharset(config->ctx, charset, buf, rlen);
    
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
