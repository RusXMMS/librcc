#ifndef _RCC_LNG_H
#define _RCC_LNG_H

#include "lngconfig.h"

rcc_language_ptr rccGetLanguagePointer(rcc_context ctx, rcc_language_id language_id);

#define rccGetEnginePointer(ctx, engine_id) rccConfigGetEnginePointer(ctx->current_config, engine_id)

#endif /* _RCC_LNG_H */
