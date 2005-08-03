#include <stdio.h>
#include <string.h>

#include <librcd.h>
#include <librcc.h>

static rcc_autocharset_id AutoengineRussian(rcc_engine_context ctx, const char *buf, int len) {
    return (rcc_autocharset_id)rcdGetRussianCharset(buf,len);
}

static rcc_engine russian_engine = {
    "Russian", NULL, NULL, &AutoengineRussian, {"CP1251","KOI8-R","UTF-8","IBM866", NULL}
};

static rcc_engine ukrainian_engine = {
    "Russian", NULL, NULL, &AutoengineRussian, {"CP1251","KOI8-U","UTF-8","IBM865", NULL}
};

rcc_engine *rccGetInfo(const char *lang) {
    if (!lang) return NULL;
    
    if (!strcmp(lang, "ru")) return &russian_engine;
    if (!strcmp(lang, "uk")) return &ukrainian_engine;
    
    return NULL;
}
