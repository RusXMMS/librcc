/*
  LibRCC - interface to enca library

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
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "plugin.h"
#include "engine.h"
#include "rccconfig.h"

#include "rccenca.h"
#ifdef RCC_ENCA_SUPPORT
# ifdef RCC_ENCA_DYNAMIC
#  include "fake_enca.h"
# else
#  include <enca.h>
# endif /* RCC_ENCA_DYNAMIC */
#endif /* RCC_ENCA_SUPPORT */

#ifdef RCC_ENCA_DYNAMIC
static rcc_library_handle enca_handle = NULL;
#endif /* RCC_ENCA_DYNAMIC */
static rcc_engine *enca_engines = NULL;


/* CORK, KEYBCS2 is missing */
rcc_enca_corrections rcc_enca_missing_corrections[] = {
    { "be", "KOI8-UNI", "ISO-IR-111" },
    { NULL, "macce", "MACCENTRALEUROPE" },
    { "zh", "HZ", "HZ" },
    { "sk", "KOI-8_CS_2", "CSKOI8R" },
    { NULL, NULL, NULL }
};

rcc_enca_corrections rcc_enca_error_corrections[] = {
    { NULL, "ECMA-cyrillic", "ISO-IR-111" },
    { NULL, NULL, NULL }
};


static const char *rccEncaGetCorrection(const char *lang, const char *charset) {
    int i;
    for (i=0;rcc_enca_error_corrections[i].enca_charset;i++) {
	if (((!rcc_enca_error_corrections[i].lang)||((lang)&&(!strcmp(lang, rcc_enca_error_corrections[i].lang))))&&(!strcmp(charset, rcc_enca_error_corrections[i].enca_charset)))
	    return rcc_enca_error_corrections[i].iconv_charset;
    }
    return charset;
}

static const char *rccEncaGetMissing(const char *lang, const char *charset) {
    int i;
    for (i=0;rcc_enca_missing_corrections[i].enca_charset;i++) {
	if (((!rcc_enca_missing_corrections[i].lang)||((lang)&&(!strcmp(lang, rcc_enca_missing_corrections[i].lang))))&&(!strcmp(charset, rcc_enca_missing_corrections[i].enca_charset)))
	    return rcc_enca_missing_corrections[i].iconv_charset;
    }
    return charset;
}


rcc_engine_internal rccEncaInitContext(rcc_engine_context ctx) {
#ifdef RCC_ENCA_SUPPORT
    EncaAnalyser enca;
    
    if ((!ctx)||(!ctx->config)) return NULL;
    
    enca = enca_analyser_alloc(rccConfigGetLanguageName(ctx->config));    
    if (!enca) return NULL;

    enca_set_threshold(enca, 1);
    enca_set_multibyte(enca, 1);
    enca_set_ambiguity(enca, 1);
    enca_set_garbage_test(enca, 0);
    enca_set_filtering(enca, 0);
    enca_set_significant(enca,1);
    enca_set_termination_strictness(enca,0);
    
    return (rcc_engine_internal)enca;
#else /* RCC_ENCA_SUPPORT */
    return NULL;
#endif /* RCC_ENCA_SUPPORT */
}

void rccEncaFreeContext(rcc_engine_context ctx) {
    rcc_engine_internal internal;
#ifdef RCC_ENCA_SUPPORT
    internal = rccEngineGetInternal(ctx);
    if (internal)
	enca_analyser_free(internal);
#endif /* RCC_ENCA_SUPPORT */
}

rcc_autocharset_id rccEnca(rcc_engine_context ctx, const char *buf, int len) {
#ifdef RCC_ENCA_SUPPORT
    rcc_engine_internal internal;
    const char *charset;
    EncaEncoding ee;

    internal = rccEngineGetInternal(ctx);
    if ((!internal)||(!buf)) return (rcc_charset_id)-1;
    
    ee = enca_analyse_const((EncaAnalyser)ctx->internal,(const unsigned char*)buf,len?len:strlen(buf));
    if (ee.charset<0) return (rcc_charset_id)-1;

    charset = enca_charset_name(ee.charset, ENCA_NAME_STYLE_ICONV);
    if (charset) {
        charset = rccEncaGetCorrection(rccEngineGetLanguage(ctx)->sn, charset);
    } else {
        charset = rccEncaGetMissing(rccEngineGetLanguage(ctx)->sn, enca_charset_name(ee.charset, ENCA_NAME_STYLE_ENCA));
    }
    return rccEngineGetAutoCharsetByName(ctx, charset);
#else /* RCC_ENCA_SUPPORT */
    return (rcc_charset_id)-1;
#endif /* RCC_ENCA_SUPPORT */
}

rcc_engine rcc_enca_engine = {
    "Enca Library", &rccEncaInitContext, &rccEncaFreeContext, &rccEnca, {"UTF-8", NULL}
};


static int rccEncaLibraryLoad() {
#ifdef RCC_ENCA_DYNAMIC
    if (enca_handle) return 0;
    
    enca_handle = rccLibraryOpen(RCC_ENCA_LIB);
    if (!enca_handle) return -1;

    enca_set_multibyte=rccLibraryFind(enca_handle,"enca_set_multibyte");
    enca_set_interpreted_surfaces=rccLibraryFind(enca_handle,"enca_set_interpreted_surfaces");
    enca_set_ambiguity=rccLibraryFind(enca_handle,"enca_set_ambiguity");
    enca_set_filtering=rccLibraryFind(enca_handle,"enca_set_filtering");
    enca_set_garbage_test=rccLibraryFind(enca_handle,"enca_set_garbage_test");
    enca_set_termination_strictness=rccLibraryFind(enca_handle,"enca_set_termination_strictness");
    enca_set_significant=rccLibraryFind(enca_handle,"enca_set_significant");
    enca_set_threshold=rccLibraryFind(enca_handle,"enca_set_threshold");
    enca_charset_name=rccLibraryFind(enca_handle,"enca_charset_name");
    enca_get_language_charsets=rccLibraryFind(enca_handle,"enca_get_language_charsets");
    enca_analyser_alloc=rccLibraryFind(enca_handle,"enca_analyser_alloc");
    enca_analyser_free=rccLibraryFind(enca_handle,"enca_analyser_free");
    enca_analyse_const=rccLibraryFind(enca_handle,"enca_analyse_const");

    if ((!enca_set_multibyte)||(!enca_set_interpreted_surfaces)||(!enca_set_ambiguity)||
	(!enca_set_filtering)||(!enca_set_garbage_test)||(!enca_set_termination_strictness)||
	(!enca_set_significant)||(!enca_set_threshold)||(!enca_charset_name)||
	(!enca_get_language_charsets)||(!enca_analyser_alloc)||(!enca_analyser_free)||
	(!enca_analyse_const)) {
	    rccLibraryClose(enca_handle);
	    enca_handle = NULL;
# ifdef RCC_DEBUG
	    perror( "rccEnca. Incomplete function set in library" );
# endif /* RCC_DEBUG */
	    return -1;
    }

#endif /* RCC_ENCA_DYNAMIC */
    return 0;
}

static void rccEncaLibraryUnload() {
#ifdef RCC_ENCA_DYNAMIC
    if (enca_handle) {
	rccLibraryClose(enca_handle);
	enca_handle = NULL;
    }
#endif /* RCC_ENCA_DYNAMIC */
}

int rccEncaInit() {
    int err;
    unsigned int i,j,k,l;

    rcc_engine **engines;
    
    int *charsets;
    size_t n_charsets;
    const char *charset;

#ifdef RCC_ENCA_SUPPORT
    if (enca_engines) return 0;
    for (i=0;rcc_default_languages[i].sn;i++);
    enca_engines = (rcc_engine*)malloc(i*sizeof(rcc_engine));
    if (!enca_engines) return -1;

    err = rccEncaLibraryLoad();
    if (err) return 0;

    for (i=0;rcc_default_languages[i].sn;i++) {
	engines = rcc_default_languages[i].engines;
	for (j=0;engines[j];j++)
	if (j >= RCC_MAX_ENGINES) continue;
	
	if (strlen(rcc_default_languages[i].sn)==2)
	    charsets = enca_get_language_charsets(rcc_default_languages[i].sn, &n_charsets);
	else
	    charsets = NULL;
	if (charsets) {
	    memcpy(enca_engines+i, &rcc_enca_engine, sizeof(rcc_engine));
	    for (k=0;enca_engines[i].charsets[k];k++);

	    if (n_charsets+k>=RCC_MAX_CHARSETS) n_charsets = RCC_MAX_CHARSETS-k;
	    
	    for (l=0;l<n_charsets;l++) {
		    // Enca bug, STYLE_ICONV return's a lot of NULL's
		charset = enca_charset_name(charsets[l], ENCA_NAME_STYLE_ICONV);
		if (charset) {
		    charset = rccEncaGetCorrection(rcc_default_languages[i].sn, charset);
		} else {
		    charset = rccEncaGetMissing(rcc_default_languages[i].sn, enca_charset_name(charsets[l], ENCA_NAME_STYLE_ENCA));
		}
		enca_engines[i].charsets[k++] = charset;
	    }
	    enca_engines[j].charsets[k] = NULL;

	    engines[j] = enca_engines + i;
	    engines[j+1] = NULL;

	    free(charsets);
	}
    }
#endif /* RCC_ENCA_SUPPORT */

    return 0;
}	    
	    
void rccEncaFree() {
#ifdef RCC_ENCA_SUPPORT
    rccEncaLibraryUnload();
    if (enca_engines) {
	free(enca_engines);
	enca_engines = NULL;
    }
#endif /* RCC_ENCA_SUPPORT */
}			    
