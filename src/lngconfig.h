#ifndef _RCC_LNGCONFIG_H
#define _RCC_LNGCONFIG_H


struct rcc_language_config_t {
    rcc_context ctx;
    rcc_language *language;
    
    rcc_engine_id engine;
    rcc_charset_id *charset;
    
    rcc_charset_id default_charset;
    unsigned char configured;
};
typedef struct rcc_language_config_t rcc_language_config_s;


rcc_engine_ptr rccConfigGetEnginePointer(rcc_language_config config, rcc_engine_id engine_id);
rcc_engine_ptr rccConfigCheckEnginePointer(rcc_language_config config, rcc_engine_id engine_id);
rcc_engine_ptr rccConfigGetCurrentEnginePointer(rcc_language_config config);
rcc_engine_ptr rccConfigCheckCurrentEnginePointer(rcc_language_config config);

int rccConfigInit(rcc_language_config config, rcc_context ctx);
int rccConfigFree(rcc_language_config config);

#endif /* _RCC_LNGCONFIG_H */
