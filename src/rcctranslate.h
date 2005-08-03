#ifndef _RCC_TRANSLATE_H
#define _RCC_TRANSLATE_H

#include "rccmutex.h"
#include "rccexternal.h"
#define RCC_EXTERNAL_COMMAND_TRANSLATE 0x80
#define RCC_EXTERNAL_COMMAND_TRANSLATE_QUEUE 0x81


struct rcc_translate_prefix_t {
    rcc_external_command_s cmd;
    unsigned long timeout;
    char from[3];
    char to[3];
    char text[1];
};
typedef struct rcc_translate_prefix_t rcc_translate_prefix_s;
typedef struct rcc_translate_prefix_t *rcc_translate_prefix;


struct rcc_translate_t {
    rcc_translate_prefix_s prefix;
    rcc_translate_prefix_s wprefix;
    size_t remaining;
    rcc_mutex mutex;
    rcc_mutex wmutex;
    int sock;

    unsigned char werror;
};
typedef struct rcc_translate_t rcc_translate_s;


int rccTranslateInit();
void rccTranslateFree();

#endif /* _RCC_TRANSLATE_H */
