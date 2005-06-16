#ifndef _RCC_ENGINE_H
#defien _RCC_ENGINE_H

int rccEngineInit(rcc_engine_context *engine_ctx, rcc_context *ctx);
void rccFreeEngine(rcc_engine_context *engine_ctx);

int rccConfigure(rcc_engine_context *ctx);

rcc_engine_internal rccEngineGetInternal(rcc_engine_context *ctx);

#endif /* _RCC_ENGINE_H */
