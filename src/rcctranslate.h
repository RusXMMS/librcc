#ifndef _RCC_TRANSLATE_H
#define _RCC_TRANSLATE_H

#include "rccexternal.h"
#define RCC_TRANSLATE_DEFAULT_TIMEOUT	5000000	/* 5s */
#define RCC_EXTERNAL_COMMAND_TRANSLATE 0x80


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
    size_t remaining;
    int sock;
};
typedef struct rcc_translate_t rcc_translate_s;


int rccTranslateInit();
void rccTranslateFree();

#endif /* _RCC_TRANSLATE_H */
