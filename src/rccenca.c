#include <stdio.h>

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

static rcc_library_handle enca_handle = NULL;
static rcc_engine *enca_engines = NULL;

rcc_engine_internal rccEncaInitContext(rcc_engine_context ctx) {
#ifdef RCC_ENCA_SUPPORT
    EncaAnalyser enca;
    
    if ((!ctx)||(!ctx->language)) return NULL;
    
    enca = enca_analyser_alloc(ctx->language->sn);    
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

rcc_charset_id rccEnca(rcc_engine_context ctx, const char *buf, int len) {
#ifdef RCC_ENCA_SUPPORT
    rcc_engine_internal internal;
    const char *charset;
    EncaEncoding ee;

    internal = rccEngineGetInternal(ctx);
    if ((!internal)||(!buf)) return -1;
    
    
    len = STRNLEN(buf, len);

    ee = enca_analyse_const((EncaAnalyser)ctx->internal,buf,len);
    if (ee.charset<0) return -1;

    charset = enca_charset_name(ee.charset, ENCA_NAME_STYLE_ICONV);
    return rccGetAutoCharsetByName(ctx->ctx, charset);
#else /* RCC_ENCA_SUPPORT */
    return -1;
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
    }

#endif /* RCC_ENCA_DYNAMIC */
    return 0;
}

static void rccEncaLibraryUnload() {
#ifdef RCC_ENCA_DYNAMIC
    if (enca_handle) {
	rccLibraryUnload(enca_handle);
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

#ifdef RCC_ENCA_SUPPORT
    if (enca_engines) return -1;
    for (i=0;rcc_default_languages[i].sn;i++);
    enca_engines = (rcc_engine*)malloc(i*sizeof(rcc_engine));
    if (!enca_engines) return -1;

    err = rccEncaLibraryLoad();
    if (err) return err;

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
	    
	    for (l=0;l<n_charsets;l++)
		enca_engines[j].charsets[k++] = enca_charset_name(charsets[l], ENCA_NAME_STYLE_ICONV);
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
