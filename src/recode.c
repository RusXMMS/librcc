/*
  LibRCC - module providing language autodetection and recoding

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */

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

typedef enum rcc_detect_language_confidence_t {
    RCC_DETECT_LANGUAGE_CONFIDENCE_UNSURE = 0,
    RCC_DETECT_LANGUAGE_CONFIDENCE_ALMOST,
    RCC_DETECT_LANGUAGE_CONFIDENCE_SURE,
    RCC_DETECT_LANGUAGE_CONFIDENCE_CACHED
} rcc_detect_language_confidence;

static rcc_language_id rccDetectLanguageInternal(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len, rcc_string *retstring, rcc_detect_language_confidence *confidence) {
    rcc_speller speller = NULL;
    long i, nlanguages;
    rcc_language_config config, config0 = NULL, config1 = NULL;
    rcc_string recoded;
    unsigned char *utf8;
    size_t j, mode;
    rcc_speller_result spres;
    unsigned long words, result, own;
    size_t longest, ownlongest;
    unsigned char english_mode, english_word = 1;
    char *english_string = NULL;
    rcc_language_id english_lang = (rcc_language_id)-1;
    size_t english_longest = 0;
    unsigned char is_english_string = 1;
    double res, ownres, english_res = 0;
    rcc_option_value usedb4;
    rcc_language_id bestlang = (rcc_language_id)-1;
    size_t bestlongest = RCC_ACCEPTABLE_LENGTH;
//    size_t bestownlongest = RCC_ACCEPTABLE_LENGTH;
    unsigned long bestown = 0;
    double bestres = RCC_ACCEPTABLE_PROBABILITY;
    char *best_string = NULL;
    rcc_language_id bestfixlang = (rcc_language_id)-1;
    unsigned long k;
    rcc_language_id *parents;
    size_t chars = 0;
    char llang[RCC_MAX_LANGUAGE_CHARS];
    rcc_language_id locale_lang;
    unsigned char defstep = 0;

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
		if (confidence) *confidence = RCC_DETECT_LANGUAGE_CONFIDENCE_CACHED;
	        return english_lang;
	    }
	}
    }
    
    if (!rccGetOption(ctx, RCC_OPTION_AUTODETECT_LANGUAGE)) return (rcc_language_id)-1;

    nlanguages = ctx->n_languages;

    english_lang = rccGetLanguageByName(ctx, rcc_english_language_sn);
    
    for (i=0;i<nlanguages;(defstep>1)?i++:i) {
	if (i) {
	    config = rccGetUsableConfig(ctx, (rcc_language_id)i);
	    if ((!config)||(config==config0)||(config==config1)) continue;
	} else {
	    switch (defstep) {
		case 0:
		    config = rccGetCurrentConfig(ctx);
		    config0 = config;
		break;
		case 1:
		    if (!rccLocaleGetLanguage(llang ,ctx->locale_variable, RCC_MAX_LANGUAGE_CHARS)) {
			locale_lang = rccGetLanguageByName(ctx, llang);
			config = rccGetConfig(ctx, locale_lang);
		    } else config = NULL;
		    config1 = config;
		break;
		default:
		    config = NULL;
	    }
	    defstep++;
	    if ((!config)||(config0==config1)) continue;
	}
	
	
	if (bestfixlang != (rcc_language_id)-1) {
	    parents = ((rcc_language_internal*)config->language)->parents;
	    for (k = 0;parents[k] != (rcc_language_id)-1;k++)
		if (parents[k] == bestfixlang) break;

	    if (parents[k] != bestfixlang) continue;
	}
	
	speller = rccConfigGetSpeller(config);
	if (rccSpellerGetError(speller)) continue;

	recoded = rccConfigSizedFrom(config, class_id, buf, len);
	if (!recoded) continue;
	
	if (!strcasecmp(config->language->sn, rcc_english_language_sn)) english_mode = 1;
	else english_mode = 0;
	
	utf8 = (unsigned char*)rccStringGetString(recoded);
	
	for (result=0,own=0,words=0,ownlongest=0,longest=0,mode=0,j=0;utf8[j];j++) {
	    if (isSpace(utf8[j])) {
		if (mode) {
		    if ((english_mode)&&(!english_word)) is_english_string = 0;
		    
		    spres = rccSpellerSized(speller, (char*)utf8 + mode - 1, j - mode + 1, 1);
		    if (rccSpellerResultIsCorrect(spres)) {
			result++;
			chars = rccStringSizedGetChars((char*)utf8 + mode - 1, j - mode + 1);
			if (chars > longest) longest = chars;
		    }
		    if (rccSpellerResultIsOwn(spres)) {
			own++;
			if (chars > ownlongest) ownlongest = chars;
		    }
#if RCC_DEBUG_LANGDETECT > 1
		    printf("%s: %u (%.*s)\n", config->language->sn, spres, j - mode + 1, utf8 + mode -1);
#endif /* RCC_DEBUG_LANGDETECT */
		    words++;
		    mode = 0;
		} else continue;
	    } else {
		if (!mode) {
		    mode = j + 1;
		    english_word = 1;
		}

		if (utf8[j]>0x7F) english_word = 0;
	    }
	}

	if (mode) {
	    if ((english_mode)&&(!english_word)) is_english_string = 0;
		    
	    spres = rccSpeller(speller, (char*)utf8 + mode - 1);
	    if (rccSpellerResultIsCorrect(spres)) {
		result++;
		chars = rccStringSizedGetChars((char*)utf8 + mode - 1, 0);
		if (chars > longest) longest = chars;
	    }
	    if (rccSpellerResultIsOwn(spres)) {
		own++;
		if (chars > ownlongest) ownlongest = chars;
	    }
#if RCC_DEBUG_LANGDETECT > 1
	    printf("%s: %u (%.*s)\n", config->language->sn, spres, j - mode + 1, utf8 + mode -1);
#endif /* RCC_DEBUG_LANGDETECT */
		    
	    words++;
	}

	if (english_mode) {
	    if (english_string) free(english_string);

	    english_res = 1.*result/words;
	    english_lang = (rcc_language_id)i;
	    english_longest = longest;
	    english_string = recoded;
	} else if (words>0) {
	    res = 1.*result/words;
	    ownres = 1.*own/words;
	    
	    if  ((res > bestres + RCC_PROBABILITY_STEP)||
		    ((res > bestres - RCC_PROBABILITY_STEP)&&(longest > bestlongest))||
		    ((res > bestres + 1E-10)&&(longest == bestlongest))||
		    (((res-bestres)<1E-10)&&((bestres-res)<1E-10)&&(longest == bestlongest)&&(own > 0))) {
		    
		if (best_string) free(best_string);
		
		bestres = res;
		bestlang = rccGetRealLanguage(ctx, (rcc_language_id)i);
		bestlongest = longest;
		best_string = recoded;
		bestown = own;
//		bestownlongest = ownlongest;
		
		if ((ownres > RCC_REQUIRED_PROBABILITY)&&(ownlongest > RCC_REQUIRED_LENGTH)) {
		    bestfixlang = bestlang;
		}
	    }  else if (!accepted_nonenglish_langs) {
		bestlang = (rcc_language_id)i;
		best_string = recoded;
	    } else free(recoded);

	    accepted_nonenglish_langs++;
	} else free(recoded);
    }

    if ((bestres > RCC_REQUIRED_PROBABILITY)&&(bestlongest > RCC_REQUIRED_LENGTH)&&(bestown>0)) {
	if (english_string) free(english_string);
	if (retstring) *retstring = best_string;
	else if (best_string) free(best_string);

	if (confidence) *confidence = RCC_DETECT_LANGUAGE_CONFIDENCE_SURE;
        return bestlang;
    } 

    if ((is_english_string)&&(english_res > RCC_REQUIRED_PROBABILITY)&&(english_longest > RCC_REQUIRED_LENGTH)) {
	if (best_string) free(best_string);
	if (retstring) *retstring = english_string;
	else if (english_string) free(english_string);
	
	if (confidence) *confidence = RCC_DETECT_LANGUAGE_CONFIDENCE_SURE;
        return english_lang;
    }

    if ((bestres > RCC_ACCEPTABLE_PROBABILITY)&&(bestlongest > RCC_ACCEPTABLE_LENGTH)&&(bestown>0)) {
	if (english_string) free(english_string);
	if (retstring) *retstring = best_string;
	else if (best_string) free(best_string);

	if (confidence) *confidence = RCC_DETECT_LANGUAGE_CONFIDENCE_ALMOST;
        return bestlang;
    } 

    if ((is_english_string)&&(((english_res > RCC_ACCEPTABLE_PROBABILITY)&&(english_longest > RCC_ACCEPTABLE_LENGTH))||(!bestown))) {
	if (best_string) free(best_string);	
	if (retstring) *retstring = english_string;
	else if (english_string) free(english_string);

	if (confidence) *confidence = RCC_DETECT_LANGUAGE_CONFIDENCE_ALMOST;
        return english_lang;
    } 
    
    if (best_string) {
	if (english_string) free(english_string);
	if (retstring) *retstring = best_string;
	else if (best_string) free(best_string);

	if (confidence) *confidence = RCC_DETECT_LANGUAGE_CONFIDENCE_UNSURE;
        return bestlang;
    } else if (best_string) free(best_string);
    
    if ((english_res > RCC_ACCEPTABLE_PROBABILITY)&&(english_longest > RCC_ACCEPTABLE_LENGTH)) {
	if (retstring) *retstring = english_string;
	else if (english_string) free(english_string);

	if (confidence) *confidence = RCC_DETECT_LANGUAGE_CONFIDENCE_UNSURE;
        return english_lang;
    } else if (english_string) free(english_string);
    
    return (rcc_language_id)-1;
}

rcc_language_id rccDetectLanguage(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    
    return rccDetectLanguageInternal(ctx, class_id, buf, len, NULL, NULL);
}


static int rccIsParrentLanguage(rcc_language_config config, rcc_language_id parent) {
    unsigned int i;
    rcc_language_id language;
    rcc_language_id *list;

    language = rccConfigGetLanguage(config);
    if (parent == language) return 1;
    
    list = ((rcc_language_internal*)config->language)->parents;
    for (i=0;list[i] != (rcc_language_id)-1;i++)
        if  (list[i] == parent) return 1;

    return 0;
}


static int rccAreRelatedLanguages(rcc_language_config c1, rcc_language_config c2) {
    rcc_language_id l1, l2;

    l1 = rccConfigGetLanguage(c1);
    l2 = rccConfigGetLanguage(c2);
    
    if (rccIsParrentLanguage(c1, l2)) return 1;
    if (rccIsParrentLanguage(c2, l1)) return 1;
    
    return 0;
}


static char *rccRecodeTranslate(rcc_language_config *config, rcc_class_id class_id, const char *utfstring) {
    rcc_context ctx;
    rcc_language_config curconfig;
    
    rcc_option_value translate;
    rcc_class_type ctype;
    rcc_language_id language_id, english_language_id, current_language_id;

    char llang[RCC_MAX_LANGUAGE_CHARS];

    rcc_translate trans, entrans;
    
    unsigned int i;
    char *translated;
    unsigned char change_case;

    ctx = (*config)->ctx;

    translate = rccGetOption(ctx, RCC_OPTION_TRANSLATE);
    if (translate == RCC_OPTION_TRANSLATE_OFF) return NULL;

    ctype = rccGetClassType(ctx, class_id);
    if ((ctype != RCC_CLASS_TRANSLATE_LOCALE)&&(ctype != RCC_CLASS_TRANSLATE_CURRENT)&&(ctype != RCC_CLASS_TRANSLATE_FROM)) return NULL;

    language_id = rccConfigGetLanguage(*config);	
	
    english_language_id = rccGetLanguageByName(ctx, rcc_english_language_sn);
    
    if ((translate == RCC_OPTION_TRANSLATE_TO_ENGLISH)||(translate == RCC_OPTION_TRANSLATE_TRANSLITERATE)) {
	current_language_id = english_language_id ;
    } else {
	if (ctype == RCC_CLASS_TRANSLATE_LOCALE) {
	    if (!rccLocaleGetLanguage(llang ,ctx->locale_variable, RCC_MAX_LANGUAGE_CHARS))
		current_language_id = rccGetLanguageByName(ctx, llang);
	    else 
		current_language_id = (rcc_language_id)-1;
	} else 
	    current_language_id = rccGetCurrentLanguage(ctx);
    }

    if (current_language_id == (rcc_language_id)-1) return NULL;
    if (language_id == current_language_id) return NULL;

    curconfig = rccGetConfig(ctx, current_language_id);
    if (!curconfig) return NULL;

    if (rccConfigConfigure(curconfig)) return NULL;

    if (translate == RCC_OPTION_TRANSLATE_TRANSLITERATE) {
	if (!strcasecmp((*config)->language->sn, rcc_russian_language_sn)) {
	    translated = rccSizedRecodeCharsets(ctx, "UTF-8", "KOI8-R", utfstring, 0, NULL);
	    if (!translated) return NULL;
	    for (i=0;translated[i];i++) {
		if (translated[i]&0x80) change_case = 1;
		else change_case = 0;
		
		translated[i]=translated[i]&0x7F;
		if (change_case) {
	    	    if ((translated[i]<'Z')&&(translated[i]>'A'))
			translated[i]=translated[i]-'A'+'a';
		    else if ((translated[i]<'z')&&(translated[i]>'a'))
			translated[i]=translated[i]-'a'+'A';
		}
	    }
	    *config = curconfig;
	    return translated;
	}
	if (!strcasecmp((*config)->language->sn, rcc_ukrainian_language_sn)) {
	    translated = rccSizedRecodeCharsets(ctx, "UTF-8", "KOI8-U", utfstring, 0, NULL);
	    if (!translated) return NULL;
	    for (i=0;translated[i];i++) {
		if (translated[i]&0x80) change_case = 1;
		else change_case = 0;
		
		translated[i]=translated[i]&0x7F;
		if (change_case) {
	    	    if ((translated[i]<'Z')&&(translated[i]>'A'))
			translated[i]=translated[i]-'A'+'a';
		    else if ((translated[i]<'z')&&(translated[i]>'a'))
			translated[i]=translated[i]-'a'+'A';
		}
	    }
	    *config = curconfig;
	    return translated;
	}

	translated = rccSizedRecodeCharsets(ctx, "UTF-8", "US-ASCII//TRANSLIT", utfstring, 0, NULL);
	if (translated) *config = curconfig;
	return translated;
    }
    
    if (translate == RCC_OPTION_TRANSLATE_SKIP_RELATED) {
	if (rccAreRelatedLanguages(curconfig, *config)) return NULL;
    }
    
    if (translate == RCC_OPTION_TRANSLATE_SKIP_PARENT) {
	if (rccIsParrentLanguage(curconfig, language_id)) return NULL;
    }

    trans = rccConfigGetTranslator(*config, current_language_id);
    if (trans) {
        translated = rccTranslate(trans, utfstring);
        if (translated) {
            if (((translate != RCC_OPTION_TRANSLATE_TO_ENGLISH))&&(!((rcc_language_internal*)curconfig->language)->latin)&&(rccIsASCII(translated))) {
	        free(translated);
	        translated = NULL;
	    }
	}
    } else translated = NULL;
    
    if ((!translated)&&(current_language_id != english_language_id)&&(!rccAreRelatedLanguages(*config, curconfig))) {
	curconfig = rccGetConfig(ctx, english_language_id);
	if (!curconfig) return NULL;
	if (rccConfigConfigure(curconfig)) return NULL;
	
	entrans = rccConfigGetEnglishTranslator(*config);
	if (entrans) translated = rccTranslate(entrans, utfstring);
    }

    if (translated) *config = curconfig;
    return translated;
}

rcc_string rccSizedFrom(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    int err;
    size_t ret;
    rcc_language_config config;
    rcc_language_id language_id, detected_language_id;
    rcc_autocharset_id charset_id;
    rcc_iconv icnv = NULL;
    rcc_string result;
    rcc_class_type class_type;
    rcc_option_value usedb4;
    const char *charset;
    char *translate = NULL;
    rcc_detect_language_confidence confidence;
    
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

    class_type = rccGetClassType(ctx, class_id);
    usedb4 = rccGetOption(ctx, RCC_OPTION_LEARNING_MODE);
    
    detected_language_id = rccDetectLanguageInternal(ctx, class_id, buf, len, &result, &confidence);
    if (detected_language_id != (rcc_language_id)-1) {
#ifdef RCC_DEBUG_LANGDETECT
	    printf("Language %i(%s): %s\n", rccStringGetLanguage(result), rccStringGetLanguage(result)?rccGetLanguageName(ctx, rccStringGetLanguage(result)):"", result);
#endif /* RCC_DEBUG_LANGDETECT */

	if ((result)&&(rccGetOption(ctx, RCC_OPTION_TRANSLATE))&&(class_type == RCC_CLASS_TRANSLATE_FROM)) {
	    rccMutexLock(ctx->mutex);
	    config = rccGetCurrentConfig(ctx);
	    translate = rccRecodeTranslate(&config, class_id, rccStringGetString(result));
	    rccMutexUnLock(ctx->mutex);
	    
	    if (translate) {
		language_id = rccConfigGetLanguage(config);
		free(result);
		result = rccCreateString(language_id, translate, 0);
	    }
	}


	if ((result)&&
	    (usedb4&RCC_OPTION_LEARNING_FLAG_LEARN)&&
	    (confidence!=RCC_DETECT_LANGUAGE_CONFIDENCE_CACHED)&&
	    ((language_id==detected_language_id)||(confidence!=RCC_DETECT_LANGUAGE_CONFIDENCE_UNSURE))&&
	    (!rccStringSetLang(result, ctx->languages[language_id]->sn))) {

	    rccDb4SetKey(ctx->db4ctx, buf, len, result);
	}

	return result;
    }


    err = rccConfigure(ctx);
    if (err) return NULL;
    
    rccMutexLock(ctx->mutex);
    if (class_type == RCC_CLASS_KNOWN) charset_id = (rcc_autocharset_id)-1;
    else charset_id = rccDetectCharset(ctx, class_id, buf, len);
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
	if (ret == (size_t)-1) {
	    rccMutexUnLock(ctx->mutex);
	    return NULL;
	}
	
	if ((rccGetOption(ctx, RCC_OPTION_TRANSLATE))&&(rccGetClassType(ctx, class_id) == RCC_CLASS_TRANSLATE_FROM)) {
	    config = rccGetCurrentConfig(ctx);
	    translate = rccRecodeTranslate(&config , class_id, ctx->tmpbuffer);
	    if (translate) language_id = rccConfigGetLanguage(config);
	}

	result = rccCreateString(language_id, translate?translate:ctx->tmpbuffer, translate?0:ret);
    } else {
	if ((rccGetOption(ctx, RCC_OPTION_TRANSLATE))&&(rccGetClassType(ctx, class_id) == RCC_CLASS_TRANSLATE_FROM)) {
	    config = rccGetCurrentConfig(ctx);
	    translate = rccRecodeTranslate(&config , class_id, buf);
	    if (translate) language_id = rccConfigGetLanguage(config);
	}

	result = rccCreateString(language_id, translate?translate:buf, translate?0:len);
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
    
    if (((class_type == RCC_CLASS_TRANSLATE_LOCALE)||(class_type == RCC_CLASS_TRANSLATE_CURRENT))&&(rccGetOption(ctx, RCC_OPTION_TRANSLATE))) {
	rccMutexLock(ctx->mutex);
	translated = rccRecodeTranslate(&config, class_id, utfstring);
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
    if ((rccGetOption(ctx, RCC_OPTION_TRANSLATE))&&((class_type == RCC_CLASS_TRANSLATE_LOCALE)||(class_type == RCC_CLASS_TRANSLATE_CURRENT))) goto recoding;

    class_type = rccGetClassType(ctx, from);
    if ((rccGetOption(ctx, RCC_OPTION_TRANSLATE))&&(class_type == RCC_CLASS_TRANSLATE_FROM)) goto recoding;

    rccMutexLock(ctx->mutex);
    if (class_type == RCC_CLASS_KNOWN) from_charset_id = (rcc_autocharset_id)-1;
    else from_charset_id = rccDetectCharset(ctx, from, buf, len);
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

	if (!result) {
	    config = rccGetCurrentConfig(ctx);
	    if (config) {
		rccMutexLock(ctx->mutex);
		rccMutexLock(config->mutex);
		result = rccFS3(config, to, prefix, rccStringGetString(string));
		rccMutexUnLock(config->mutex);
		rccMutexUnLock(ctx->mutex);
	    }
	}

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

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

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

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

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

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    
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

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    
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
