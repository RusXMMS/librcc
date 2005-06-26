#ifndef _RCC_ENGINE_H
#define _RCC_ENGINE_H

struct rcc_engine_context_t {
    rcc_context ctx;
    rcc_language *language;
    
    rcc_engine_function func;
    rcc_engine_free_function free_func;

    rcc_engine_internal internal;
};
typedef struct rcc_engine_context_t rcc_engine_context_s;

int rccEngineInit(rcc_engine_context engine_ctx, rcc_context ctx);
void rccEngineFree(rcc_engine_context engine_ctx);

rcc_engine_internal rccEngineGetInternal(rcc_engine_context ctx);

#endif /* _RCC_ENGINE_H */
