#include <stdio.h>
#include <string.h>

#include "internal.h"

int rccEngineInit(rcc_engine_context engine_ctx, rcc_context ctx) {
    if ((!ctx)||(!engine_ctx)) return -1;
    
    engine_ctx->ctx = ctx;
    engine_ctx->free_func = NULL;
    engine_ctx->func = NULL;
    return 0;
}

void rccEngineFree(rcc_engine_context engine_ctx) {
    if (!engine_ctx) return;

    if (engine_ctx->free_func) {
	engine_ctx->free_func(engine_ctx);
	engine_ctx->free_func = NULL;
    }

    engine_ctx->func = NULL;
    engine_ctx->internal = NULL;
}

int rccEngineConfigure(rcc_engine_context ctx) {
    rcc_language_id language_id;
    rcc_engine_id engine_id;
    rcc_engine *engine;

    if ((!ctx)||(!ctx->ctx)) return -1;

    rccEngineFree(ctx);
    
    language_id = rccGetCurrentLanguage(ctx->ctx);
    if (language_id == (rcc_language_id)-1) return -1;

    engine_id = rccGetCurrentEngine(ctx->ctx);
    if (engine_id == (rcc_engine_id)-1) return -1;
    
    engine = ctx->ctx->languages[language_id]->engines[engine_id];
    
    ctx->free_func = engine->free_func;
    ctx->func = engine->func;
    ctx->language = ctx->ctx->languages[language_id];

    if (engine->init_func) ctx->internal = engine->init_func(ctx);
    else ctx->internal = NULL;
    
    return 0;
}

rcc_engine_internal rccEngineGetInternal(rcc_engine_context ctx) {
    if (!ctx) return NULL;

    return ctx->internal;
}
