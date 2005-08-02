#ifndef _RCC_LNG_H
#define _RCC_LNG_H

#include "internal.h"
#include "lngconfig.h"


int rccCheckLanguageUsability(rcc_context ctx, rcc_language_id language_id);
rcc_language_ptr rccGetLanguagePointer(rcc_context ctx, rcc_language_id language_id);

#define rccGetCurrentEnginePointer(ctx) rccConfigGetCurrentEnginePointer(ctx->current_config)
#define rccCheckCurrentEnginePointer(ctx) rccConfigCheckCurrentEnginePointer(ctx->current_config)
#define rccGetEnginePointer(ctx,id) rccConfigGetEnginePointer(ctx->current_config, id)
#define rccCheckEnginePointer(ctx,id) rccConfigCheckEnginePointer(ctx->current_config, id)

#endif /* _RCC_LNG_H */
