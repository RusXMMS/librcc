#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#include "internal.h"
#include "rcciconv.h"
#include "fs.h"
#include "lng.h"
#include "rccstring.h"
#include "rccconfig.h"
#include "rccdb4.h"
#include "rcctranslate.h"
#include "rccspell.h"

#define isSpace(ch) ((ch<0x7F)&&((ch<'A')||(ch>'z')||((ch>'Z')&&(ch<'a'))))
#define RCC_REQUIRED_PROBABILITY	0.66

rcc_language_id rccDetectLanguageInternal(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len, rcc_string *retstring) {
    rcc_speller speller;
    unsigned long i, nlanguages;
    rcc_language_config config, config0 = NULL;
    rcc_string recoded;
    unsigned char *utf8;
    size_t j, mode;
    unsigned long words, english, result;
    unsigned char english_mode, english_word = 1;
    rcc_language_id english_lang = (rcc_language_id)-1;
    double res, english_res = 0;
    rcc_option_value usedb4;
    

    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);

    if (usedb4&RCC_OPTION_LEARNING_FLAG_USE) {
	recoded = rccDb4GetKey(ctx->db4ctx, buf, len);
	if (recoded) {
	     if (rccStringFixID(recoded, ctx)) free(recoded);
	     else {
	        english_lang = rccStringGetLanguage(recoded);
	        if (retstring) *retstring = recoded;
		else free(recoded);
	        return english_lang;
	    }
	}
    }
    
    if (!rccGetOption(ctx, RCC_OPTION_AUTODETECT_LANGUAGE)) return (rcc_language_id)-1;

    nlanguages = ctx->n_languages;

    for (i=0;i<nlanguages;i++) {
	config = rccGetUsableConfig(ctx, (rcc_language_id)i);
	if (!config) continue;

	if (i) {
	    if (config==config0) continue;
	} else config0=config;
	
	speller = rccConfigGetSpeller(config);
	if (rccSpellerGetError(speller)) continue;

	recoded = rccConfigSizedFrom(config, class_id, buf, len);
	if (!recoded) continue;
	
	if (!strcasecmp(config->language->sn, rcc_english_language_sn)) english_mode = 1;
	else english_mode = 0;
	
	utf8 = (char*)rccStringGetString(recoded);
	for (result=0,english=0,words=0,mode=0,j=0;utf8[j];j++) {
	    if (isSpace(utf8[j])) {
		if (mode) {
		    if ((!english_mode)&&(english_word)) english++;
		    result+=rccSpellerSized(speller, utf8 + mode - 1, j - mode + 1)?1:0;
		    words++;
		    mode = 0;
		} else continue;
	    } else {
		if (mode) {
		    if (utf8[j]>0x7F) english_word = 0;
		} else {
		    mode = j + 1;
		    english_word = 1;
		}
	    }
	}
	if (mode) {
	    result+=rccSpeller(speller, utf8 + mode - 1)?1:0;
	    words++;
	}
	
	if (english_mode) {
	    english_res = 1.*result/words;
	    english_lang = (rcc_language_id)i;    
	} else if (words) {
	    res = 1.*result/words;
	    if (res > RCC_REQUIRED_PROBABILITY) {
		if (retstring) *retstring = recoded;
		else free(recoded);
		return (rcc_language_id)i;
	    }
	    if (words > english) {
		res = 1.*(result - english)/(words - english);
		if (res > RCC_REQUIRED_PROBABILITY) {
		    if (retstring) *retstring = recoded;
		    else free(recoded);
		    return (rcc_language_id)i;
		}
	    }
	}
	
	free(recoded);
    }

    if (english_res > RCC_REQUIRED_PROBABILITY) {
        if (retstring) {
	    *retstring = rccCreateString(english_lang, buf, len);
	}
        return english_lang;
    }
    
    return (rcc_language_id)-1;
}


rcc_language_id rccDetectLanguage(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    
    return rccDetectLanguageInternal(ctx, class_id, buf, len, NULL);
}


rcc_autocharset_id rccConfigDetectCharset(rcc_language_config config, rcc_class_id class_id, const char *buf, size_t len) {
    rcc_context ctx;
    rcc_class_type class_type;
    rcc_engine_ptr engine;
    
    if ((!buf)||(!config)) return (rcc_autocharset_id)-1;
    
    ctx = config->ctx;
    
    class_type = rccGetClassType(ctx, class_id);
    if ((class_type != RCC_CLASS_FS)||((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_TITLES)))) {
	engine = rccConfigGetCurrentEnginePointer(config);
	if ((!engine)||(!engine->func)) return (rcc_autocharset_id)-1;
	return engine->func(&ctx->engine_ctx, buf, len);
    }
    
    return (rcc_autocharset_id)-1;
}

int rccDetectCharset(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    return rccConfigDetectCharset(ctx->current_config, class_id, buf, len);
}


rcc_string rccSizedFrom(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    int err;
    size_t ret;
    rcc_language_id language_id, detected_language_id;
    rcc_autocharset_id charset_id;
    rcc_iconv icnv = NULL;
    rcc_string result;
    rcc_option_value usedb4;
    const char *charset;
    
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
    if (!strcasecmp(ctx->languages[language_id]->sn, rcc_disabled_language_sn)) return NULL;


    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);
/*
    if (usedb4&RCC_OPTION_LEARNING_FLAG_USE) {
	result = rccDb4GetKey(ctx->db4ctx, buf, len);
	if (result) {
	     if (rccStringFixID(result, ctx)) free(result);
	     else return result;
	}
    }
    
    if (rccGetOption(ctx, RCC_OPTION_AUTODETECT_LANGUAGE)) {
	detected_language_id = rccDetectLanguageInternal(ctx, class_id, buf, len);
	if (detected_language_id != (rcc_language_id)-1)
	    language_id = detected_language_id;
    }
*/
    
    detected_language_id = rccDetectLanguageInternal(ctx, class_id, buf, len, &result);
    if (detected_language_id != (rcc_language_id)-1) return result;

    
    err = rccConfigure(ctx);
    if (err) return NULL;
    
    charset_id = rccDetectCharset(ctx, class_id, buf, len);
    if (charset_id != (rcc_autocharset_id)-1) {
	icnv = ctx->iconv_auto[charset_id];
	if (rccGetOption(ctx, RCC_OPTION_AUTOENGINE_SET_CURRENT)) {
	    charset = rccGetAutoCharsetName(ctx, charset_id);
	    rccSetCharsetByName(ctx, class_id, charset);
	}
    }
    else icnv = ctx->iconv_from[class_id];

    if (icnv) {
	ret = rccIConvInternal(ctx, icnv, buf, len);
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
    char *translated = NULL;
    rcc_language_config config;
    rcc_language_id language_id;
    rcc_language_id current_language_id;
    rcc_class_type class_type;
    rcc_option_value translate;
    const char *langname;
    unsigned char english_source;
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
    translate = rccGetOption(ctx, RCC_OPTION_TRANSLATE);
    
    langname = rccGetLanguageName(ctx, language_id);
    if (strcasecmp(langname, rcc_english_language_sn)) english_source = 0;
    else english_source = 1;
    
    if ((class_type != RCC_CLASS_FS)&&((translate==RCC_OPTION_TRANSLATE_FULL)||((translate)&&(!english_source)))) {
	current_language_id = rccGetCurrentLanguage(ctx);
	if (current_language_id != language_id) {
	    if ((config->trans)&&(config->translang != current_language_id)) {
		rccTranslateClose(config->trans);
		config->trans = NULL;
	    }
	    
	    if (translate != RCC_OPTION_TRANSLATE_TO_ENGLISH) {
		if (!config->trans) {
		    config->trans = rccTranslateOpen(rccGetLanguageName(ctx, language_id), rccGetLanguageName(ctx, current_language_id));
		    config->translang = current_language_id;
		}

		if (config->trans) {
		    translated = rccTranslate(config->trans, utfstring);
		    if (translated) {
			language_id = current_language_id;
		    
			config = rccGetConfig(ctx, language_id);
			if (!config) {
			    free(translated);
			    return NULL;
			}

			err = rccConfigConfigure(config);
			if (err) {
			    free(translated);
			    return NULL;
			}
		    } 
		}
	    }
	    
	    if ((translate == RCC_OPTION_TRANSLATE_TO_ENGLISH)||((config->trans)&&(!translated))) {
		puts("entrans");
		if (!config->entrans) {
		    config->entrans = rccTranslateOpen(rccGetLanguageName(ctx, language_id), rcc_english_language_sn);
		}
		    
		if (config->entrans) {
		    translated = rccTranslate(config->entrans, utfstring);

		    config = rccGetConfig(ctx, language_id);
		    if (!config) return translated;

		    err = rccConfigConfigure(config);
		    if (err) return translated;
		}
	    }
	}
    }
    
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
	newlen = rccIConvInternal(ctx, icnv, translated?translated:utfstring, translated?0:newlen);
	if (translated) free(translated);
	if (newlen == (size_t)-1) return NULL;

	result = rccCreateResult(ctx, newlen);
	if (rlen) *rlen = newlen;
    } else {
	if (translated) {
	    result = translated;
	    if (rlen) *rlen = strlen(result);
	} else {
	    result = rccStringExtractString(buf);
	    if (rlen) *rlen = newlen;
	}
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
    
    from_charset_id = rccDetectCharset(ctx, from, buf, len);
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
	res = rccIConvInternal(ctx, icnv, buf, len);
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
	res = rccIConvInternal(ctx, icnv, rccStringGetString(buf), res);
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
	res = rccIConvInternal(ctx, icnv, str, 0);
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

/* Convert to class_id from Charset.
Usage of this function assuming the knowledge about the incoming string. 
The charset as well as the language. So no detection (DB4,Aspell) of language
will be performed. */
char *rccSizedRecodeFromCharset(rcc_context ctx, rcc_class_id class_id, const char *charset, const char *buf, size_t len, size_t *rlen) {
    size_t res;
    rcc_iconv icnv;
    rcc_string str;
    char *extracted;

    if (!charset) return NULL;
    
    icnv = rccIConvOpen("UTF-8", charset);
    if (icnv) {
	res = rccIConvInternal(ctx, icnv, buf, len);
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

    res = rccIConvInternal(ctx, icnv, buf, len);
    rccIConvClose(icnv);
    
    if (res == (size_t)-1) return NULL;
    
    

    str = (char*)malloc((res+1)*sizeof(char));
    if (!str) return NULL;
    memcpy(str, ctx->tmpbuffer, res);
    if (rlen) *rlen = res;

    return str;
}
