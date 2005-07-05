#include <stdio.h>
#include "internal.h"

rcc_language_ptr *rccGetLanguageList(rcc_context ctx) {
    if (!ctx) return NULL;
    return ctx->languages;
}

rcc_charset *rccGetCharsetList(rcc_context ctx, rcc_language_id language_id) {
    if ((!ctx)||(language_id<0)||(language_id>=ctx->n_languages)) return NULL;
    if (!language_id) language_id = rccGetCurrentLanguage(ctx);

    return ctx->languages[language_id]->charsets;
}

rcc_engine_ptr *rccGetEngineList(rcc_context ctx, rcc_language_id language_id) {
    if ((!ctx)||(language_id<0)||(language_id>=ctx->n_languages)) return NULL;
    if (!language_id) language_id = rccGetCurrentLanguage(ctx);

    return ctx->languages[language_id]->engines;
}

rcc_charset *rccGetCurrentCharsetList(rcc_context ctx) {
    rcc_language_id language_id;

    if (!ctx) return NULL;

    language_id = rccGetCurrentLanguage(ctx);
    if (language_id<0) return NULL;

    return rccGetCharsetList(ctx, language_id);
}

rcc_engine_ptr *rccGetCurrentEngineList(rcc_context ctx) {
    rcc_language_id language_id;

    if (!ctx) return NULL;

    language_id = rccGetCurrentLanguage(ctx);
    if (language_id<0) return NULL;

    return rccGetEngineList(ctx, language_id); 
}

rcc_charset *rccGetCurrentAutoCharsetList(rcc_context ctx) {
    rcc_language_id language_id;
    rcc_engine_id engine_id;

    if (!ctx) return NULL;

    language_id = rccGetCurrentLanguage(ctx);
    engine_id = rccGetCurrentEngine(ctx);
    if ((language_id<0)||(engine_id<0)) return NULL;
    
    
    return ctx->languages[language_id]->engines[engine_id]->charsets;
}

rcc_class_ptr *rccGetClassList(rcc_context ctx) {
    if (!ctx) return NULL;
    
    return ctx->classes;
}
