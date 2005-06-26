#ifndef _RCC_LNGCONFIG_H
#define _RCC_LNGCONFIG_H

struct rcc_language_config_t {
    rcc_context ctx;
    rcc_language *language;
    
    rcc_engine_id engine;
    rcc_charset_id *charset;
    rcc_option_value options[RCC_MAX_OPTIONS];
};
typedef struct rcc_language_config_t rcc_language_config_s;

rcc_engine_ptr rccConfigGetEnginePointer(rcc_language_config config, rcc_engine_id engine_id);

#endif /* _RCC_LNGCONFIG_H */
