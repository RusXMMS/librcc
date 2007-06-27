#include <stdio.h>
#include <string.h>

#include <librcd.h>
#include <librcc.h>

static rcc_autocharset_id AutoengineRussian(rcc_engine_context ctx, const char *buf, int len) {
    return (rcc_autocharset_id)rcdGetRussianCharset(buf,len);
}

static rcc_engine russian_engine = {
    "LibRCD", NULL, NULL, &AutoengineRussian, {"CP1251","KOI8-R","UTF-8","IBM866", "ISO8859-1", NULL}
};

static rcc_engine ukrainian_engine = {
    "LibRCD", NULL, NULL, &AutoengineRussian, {"CP1251","KOI8-U","UTF-8","IBM865", "ISO8859-1", NULL}
};

static rcc_engine belarussian_engine = {
    "LibRCD", NULL, NULL, &AutoengineRussian, {"CP1251","ISO-IR-111","UTF-8","IBM866", "ISO8859-1", NULL}
};

rcc_engine *rccGetInfo(const char *lang) {
    if (!lang) return NULL;
    
    if (!strcmp(lang, "ru")) return &russian_engine;
    if (!strcmp(lang, "uk")) return &ukrainian_engine;
    if (!strcmp(lang, "be")) return &belarussian_engine;
    
    return NULL;
}
