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
#define RCC_PROBABILITY_STEP		0.10
#define RCC_REQUIRED_PROBABILITY	0.33
#define RCC_REQUIRED_LENGTH		5
#define RCC_ACCEPTABLE_PROBABILITY	0
#define RCC_ACCEPTABLE_LENGTH		3

static rcc_language_id rccDetectLanguageInternal(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len, rcc_string *retstring) {
    rcc_speller speller = NULL, english_speller = NULL;
    unsigned long i, nlanguages;
    rcc_language_config config, config0 = NULL;
    rcc_string recoded;
    unsigned char *utf8;
    size_t j, mode;
    unsigned long spres, words, english, result;
    size_t longest;
    unsigned char english_mode, english_word = 1;
    char *english_string = NULL;
    rcc_language_id english_lang = (rcc_language_id)-1;
    size_t english_longest = 0;
    unsigned char is_english_string = 1;
    double res, english_res = 0;
    rcc_option_value usedb4;
    rcc_language_id bestlang = (rcc_language_id)-1;
    unsigned long bestlongest = RCC_ACCEPTABLE_LENGTH;
    double bestres = RCC_ACCEPTABLE_PROBABILITY;
    char *best_string = NULL;

    unsigned long accepted_nonenglish_langs = 0;

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

    english_lang = rccGetLanguageByName(ctx, rcc_english_language_sn);
    if (english_lang != (rcc_language_id)-1) {
	config = rccGetUsableConfig(ctx, english_lang);
	if (config) {
	    english_speller  = rccConfigGetSpeller(config);
	    if (rccSpellerGetError(english_speller)) english_speller = NULL;
	}
    }
    
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
	
	for (result=0,english=0,words=0,longest=0,mode=0,j=0;utf8[j];j++) {
	    if (isSpace(utf8[j])) {
		if (mode) {
		    if ((!english_mode)&&(english_word)&&(rccSpellerSized(english_speller, utf8 + mode -1, j - mode + 1)))
			english++;
		    else {
			if ((english_mode)&&(!english_word)) is_english_string = 0;
			spres = rccSpellerSized(speller, utf8 + mode - 1, j - mode + 1)?1:0;
			if ((spres)&&((j - mode + 1)>longest)) longest = j - mode + 1;
			result+=spres;
		    }
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
	    if ((!english_mode)&&(english_word)&&(rccSpeller(english_speller, utf8 + mode -1)))
		english++;
	    else {
		if ((english_mode)&&(!english_word)) is_english_string = 0;
		spres = rccSpeller(speller, utf8 + mode - 1)?1:0;
		if ((spres)&&((j-mode+1)>longest)) longest = j - mode + 1;
		result += spres;
	    }
	    words++;
	}
	
	if (english_mode) {
	    if (english_string) free(english_string);

	    english_res = 1.*result/words;
	    english_lang = (rcc_language_id)i;
	    english_longest = longest;
	    english_string = recoded;
	} else if (words>english) {
	    res = 1.*result/(words - english);
	    if ((res > RCC_REQUIRED_PROBABILITY)&&(longest > RCC_REQUIRED_LENGTH)) {
		if (best_string) free(best_string);
		if (english_string) free(english_string);
		
		if (retstring) *retstring = recoded;
		else free(recoded);
		return (rcc_language_id)i;
	    } else if  ((res > bestres + RCC_PROBABILITY_STEP)||
		    ((res > bestres - RCC_PROBABILITY_STEP)&&(longest > bestlongest))||
		    ((res > bestres)&&(longest == bestlongest))) {
		
		if (best_string) free(best_string);
		
		bestres = res;
		bestlang = (rcc_language_id)i;
		bestlongest = longest;
		best_string = recoded;
	    }  else if (!accepted_nonenglish_langs) {
		bestlang = (rcc_language_id)i;
		best_string = recoded;
	    } else free(recoded);

	    accepted_nonenglish_langs++;
	} else free(recoded);
    }

    if ((is_english_string)&&(english_res > RCC_REQUIRED_PROBABILITY)&&(english_longest > RCC_REQUIRED_LENGTH)) {
	if (best_string) free(best_string);
	if (retstring) *retstring = english_string;
	else if (english_string) free(english_string);
        return english_lang;
    }

    if ((bestres > RCC_ACCEPTABLE_PROBABILITY)&&(bestlongest > RCC_ACCEPTABLE_LENGTH)) {
	if (english_string) free(english_string);
	if (retstring) *retstring = best_string;
	else if (best_string) free(best_string);
        return bestlang;
    } 

    if ((is_english_string)&&(english_res > RCC_ACCEPTABLE_PROBABILITY)&&(english_longest > RCC_ACCEPTABLE_LENGTH)) {
	if (best_string) free(best_string);	
	if (retstring) *retstring = english_string;
	else if (english_string) free(english_string);
        return english_lang;
    } 
    
    if (best_string) {
	if (english_string) free(english_string);
	if (retstring) *retstring = best_string;
	else if (best_string) free(best_string);
        return bestlang;
    } else if (best_string) free(best_string);
    
    if ((english_res > RCC_ACCEPTABLE_PROBABILITY)&&(english_longest > RCC_ACCEPTABLE_LENGTH)) {
	if (retstring) *retstring = english_string;
	else if (english_string) free(english_string);
        return english_lang;
    } else if (english_string) free(english_string);
    
    return (rcc_language_id)-1;
}


rcc_language_id rccDetectLanguage(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    
    return rccDetectLanguageInternal(ctx, class_id, buf, len, NULL);
}


static rcc_autocharset_id rccConfigDetectCharsetInternal(rcc_language_config config, rcc_class_id class_id, const char *buf, size_t len) {
    int err;
    rcc_context ctx;
    rcc_class_type class_type;
    rcc_engine_ptr engine;
    rcc_autocharset_id autocharset_id;
    
    if ((!buf)||(!config)) return (rcc_autocharset_id)-1;
    
    ctx = config->ctx;

    err = rccConfigConfigure(config);
    if (err) return (rcc_autocharset_id)-1;
    
    class_type = rccGetClassType(ctx, class_id);
    if ((class_type != RCC_CLASS_FS)||((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_TITLES)))) {
	rccMutexLock(config->mutex);
	engine = rccConfigGetCurrentEnginePointer(config);
	if ((engine)&&(engine->func)) autocharset_id = engine->func(&config->engine_ctx, buf, len);
	else autocharset_id = (rcc_autocharset_id)-1;
	rccMutexUnLock(config->mutex);
	return autocharset_id;
    }
    
    return (rcc_autocharset_id)-1;
}


rcc_autocharset_id rccConfigDetectCharset(rcc_language_config config, rcc_class_id class_id, const char *buf, size_t len) {
    return rccConfigDetectCharsetInternal(config, class_id, buf, len);
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
    if (detected_language_id != (rcc_language_id)-1) {
	/*printf("Language %i: %s\n", rccStringGetLanguage(result), result);*/
	return result;
    }


    err = rccConfigure(ctx);
    if (err) return NULL;
    
    rccMutexLock(ctx->mutex);
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
    rccMutexUnLock(ctx->mutex);

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
    const char *utfstring;
    char *translated = NULL;
    rcc_language_config config;
    rcc_language_id language_id;
    rcc_language_id current_language_id;
    rcc_class_type class_type;
    rcc_option_value translate;
    rcc_translate trans, entrans;
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
	rccMutexLock(ctx->mutex);
	
	current_language_id = rccGetCurrentLanguage(ctx);
	if (current_language_id != language_id) {
	    if (translate != RCC_OPTION_TRANSLATE_TO_ENGLISH) {
		trans = rccConfigGetTranslator(config, current_language_id);
		if (trans) {
		    translated = rccTranslate(trans, utfstring);
		    if (translated) {
			language_id = current_language_id;
		    
			config = rccGetConfig(ctx, language_id);
			if (!config) {
			    rccMutexUnLock(ctx->mutex);
			    free(translated);
			    return NULL;
			}

			err = rccConfigConfigure(config);
			if (err) {
			    rccMutexUnLock(ctx->mutex);
			    free(translated);
			    return NULL;
			}
		    } 
		}
	    }
	    
	    if ((translate == RCC_OPTION_TRANSLATE_TO_ENGLISH)||((config->trans)&&(!translated))) {
		entrans = rccConfigGetEnglishTranslator(config);
		if (entrans) {
		    translated = rccTranslate(config->entrans, utfstring);

		    config = rccGetConfig(ctx, language_id);
		    if (!config) {
			rccMutexUnLock(ctx->mutex);
			return translated;
		    }

		    err = rccConfigConfigure(config);
		    if (err) {
			rccMutexUnLock(ctx->mutex);
			return translated;
		    }
		}
	    }
	}
	rccMutexUnLock(ctx->mutex);
    }
    
    if ((class_type == RCC_CLASS_FS)&&(rccGetOption(ctx, RCC_OPTION_AUTODETECT_FS_NAMES))) {
	result = rccFS5(ctx, config, class_id, utfstring);
	if (result) {
	    if (rlen) *rlen = strlen(result);
	    return result;
	}
    }

    rccMutexLock(ctx->mutex);
    rccMutexLock(config->mutex);
    icnv =  config->iconv_to[class_id];
    if (icnv) {
	newlen = rccIConvInternal(ctx, icnv, translated?translated:utfstring, translated?0:newlen);
	if (translated) free(translated);
	if (newlen == (size_t)-1) result = NULL;
	else {
	    result = rccCreateResult(ctx, newlen);
	    if (rlen) *rlen = newlen;
	}
    } else {
	if (translated) {
	    result = translated;
	    if (rlen) *rlen = strlen(result);
	} else {
	    result = rccStringExtractString(buf);
	    if (rlen) *rlen = newlen;
	}
    }    
    rccMutexUnLock(config->mutex);
    rccMutexUnLock(ctx->mutex);
    
    return result;
}

char *rccSizedRecode(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *buf, size_t len, size_t *rlen) {
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
    if (rccGetOption(ctx, RCC_OPTION_LEARNING_MODE)) goto recoding;
    if (rccGetOption(ctx, RCC_OPTION_AUTODETECT_LANGUAGE)) goto recoding;
    if (rccGetOption(ctx, RCC_OPTION_TRANSLATE)) goto recoding;

    rccMutexLock(ctx->mutex);
    from_charset_id = rccDetectCharset(ctx, from, buf, len);
    if (from_charset_id != (rcc_charset_id)-1) {
	from_charset = rccGetAutoCharsetName(ctx, from_charset_id);
	to_charset = rccGetCurrentCharsetName(ctx, to);
	rccMutexUnLock(ctx->mutex);
	if ((from_charset)&&(to_charset)&&(!strcasecmp(from_charset, to_charset))) return NULL;
    } else {
	from_charset_id = rccGetCurrentCharset(ctx, from);
	to_charset_id = rccGetCurrentCharset(ctx, to);
	rccMutexUnLock(ctx->mutex);
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

    rccMutexLock(config->mutex);
    err = rccFS1(config, fspath, &prefix, &name);
    rccMutexUnLock(config->mutex);
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
	if (config) {
	    rccMutexLock(ctx->mutex);
	    rccMutexLock(config->mutex);
	    result = rccFS3(config, to, prefix, rccStringGetString(string));
	    rccMutexUnLock(config->mutex);
	    rccMutexUnLock(ctx->mutex);
	} else result = NULL;
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
    rcc_string ret;
    
    if ((!buf)||(!charset)) return NULL;
    
    config = rccGetCurrentConfig(ctx);
    if (!config) return NULL;
    
    language_id = rccConfigGetLanguage(config);
    
    icnv = rccIConvOpen("UTF-8", charset);
    if (icnv) {
	rccMutexLock(ctx->mutex);
	res = rccIConvInternal(ctx, icnv, buf, len);
	rccIConvClose(icnv);
	if (res == (size_t)-1) ret = NULL;
	else ret = rccCreateString(language_id, ctx->tmpbuffer, res);
	rccMutexUnLock(ctx->mutex);
    } else ret = rccCreateString(language_id, buf, len);
    
    return ret;
}

char *rccSizedToCharset(rcc_context ctx, const char *charset, rcc_const_string buf, size_t *rlen) {
    char *ret;
    rcc_iconv icnv;
    size_t res;
    
    if ((!buf)||(!charset)) return NULL;

    res = rccStringCheck(buf);
    if (!res) return NULL;
    
    icnv = rccIConvOpen(charset, "UTF-8");
    if (icnv) {
	rccMutexLock(ctx->mutex);
	res = rccIConvInternal(ctx, icnv, rccStringGetString(buf), res);
	rccIConvClose(icnv);
	if (res == (size_t)-1) ret = NULL;
	else {
	    ret = rccCreateResult(ctx, res);
	    if (rlen) *rlen = res;
	}
	rccMutexUnLock(ctx->mutex);

	return ret;
    } 

    if (rlen) *rlen = res;
    return rccStringExtractString(buf);
}

/* Convert from class_id to Charset */
char *rccSizedRecodeToCharset(rcc_context ctx, rcc_class_id class_id, const char *charset, rcc_const_string buf, size_t len, size_t *rlen) {
    size_t res;
    rcc_iconv icnv;
    char *ret;
    const char *str;
    char *utf8, *extracted;

    if (!charset) return NULL;
    
    utf8 = rccSizedFrom(ctx, class_id, buf, len);
    if (!utf8) return utf8;
    
    str = rccStringGetString(utf8);

    icnv = rccIConvOpen(charset, "UTF-8");
    if (icnv) {
	rccMutexLock(ctx->mutex);
	res = rccIConvInternal(ctx, icnv, str, 0);
	rccIConvClose(icnv);
	free(utf8);

	if (res == (size_t)-1) ret = NULL;
	else {
	    ret = rccCreateResult(ctx, res);
	    if (rlen) *rlen = res;
	}
	rccMutexUnLock(ctx->mutex);
	return ret;
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
	rccMutexLock(ctx->mutex);
	res = rccIConvInternal(ctx, icnv, buf, len);
	rccIConvClose(icnv);

	if (res == (size_t)-1) str = NULL;
	else str = rccCreateString(rccGetCurrentLanguage(ctx), ctx->tmpbuffer, res);
	rccMutexUnLock(ctx->mutex);
    } else str = rccCreateString(rccGetCurrentLanguage(ctx), buf, len);

    if (!str) return NULL;

    extracted = rccSizedTo(ctx, class_id, str, rlen);
    free(str);

    return extracted;
}

char *rccSizedRecodeCharsets(rcc_context ctx, const char *from, const char *to, const char *buf, size_t len, size_t *rlen) {
    char *str;
    rcc_iconv icnv;

    icnv = rccIConvOpen(to, from);
    if (!icnv) return NULL;
    str = rccIConv(icnv, buf, len, rlen);
    rccIConvClose(icnv);
    
    return str;
}
