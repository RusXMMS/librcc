#include <stdio.h>
#include <strings.h>

#include "internal.h"
#include "rccconfig.h"
#include "engine.h"
#include "opt.h"

rcc_language_alias rcc_default_aliases[] = {
    { "cs_SK", "sk" },
    { "ru_UA", "uk" },
    { NULL, NULL}
};

const char rcc_default_charset[] = "Default";
const char rcc_utf8_charset[] = "UTF-8";
const char rcc_engine_nonconfigured[] = "Default";
const char rcc_option_nonconfigured[] = "DEFAULT";

rcc_engine rcc_default_engine = {
    "Off", NULL, NULL, NULL, {NULL}
};

rcc_engine rcc_russian_engine = {
    "Russian", NULL, NULL, &rccAutoengineRussian, {"CP1251","KOI8-R","UTF-8","IBM866", NULL}
};

rcc_engine rcc_ukrainian_engine = {
    "Russian", NULL, NULL, &rccAutoengineRussian, {"CP1251","KOI8-U","UTF-8","IBM865", NULL}
};

rcc_language rcc_default_languages[RCC_MAX_LANGUAGES + 1];

rcc_language rcc_default_languages_embeded[RCC_MAX_LANGUAGES + 1] = {
{"default", {rcc_default_charset, NULL}, {
    &rcc_default_engine,
    NULL
}},
{"off", {rcc_default_charset, NULL}, {
    &rcc_default_engine,
    NULL
}},
{"en", {rcc_default_charset, rcc_utf8_charset, NULL}, {
    &rcc_default_engine,
    NULL
}},
{"ru", {rcc_default_charset,"KOI8-R","CP1251",rcc_utf8_charset,"IBM866","MACCYRILLIC","ISO8859-5", NULL}, {
    &rcc_default_engine,
#ifdef RCC_RCD_SUPPORT
    &rcc_russian_engine,
#endif /* RCC_RCD_SUPPORT */
    NULL
}},
{"uk", {rcc_default_charset,"KOI8-U","CP1251",rcc_utf8_charset,"IBM855","MACCYRILLIC","ISO8859-5","CP1125", NULL}, {
    &rcc_default_engine,
#ifdef RCC_RCD_SUPPORT
    &rcc_ukrainian_engine,
#endif /* RCC_RCD_SUPPORT */
    NULL
}},
{"be", {rcc_default_charset, rcc_utf8_charset, "CP1251", "IBM866", "ISO-8859-5", "KOI8-UNI", "maccyr" "IBM855", NULL},{
    &rcc_default_engine,
    NULL
}},
{"bg", {rcc_default_charset, rcc_utf8_charset, "CP1251", "ISO-8859-5", "IBM855", "maccyr", "ECMA-113", NULL},{
    &rcc_default_engine,
    NULL
}},
{"cz", {rcc_default_charset, rcc_utf8_charset, "ISO-8859-2", "CP1250", "IBM852", "KEYBCS2", "macce", "KOI-8_CS_2", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"es", {rcc_default_charset, rcc_utf8_charset, "ISO-8859-4", "CP1257", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"hr", {rcc_default_charset, rcc_utf8_charset, "CP1250", "ISO-8859-2", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"hu", {rcc_default_charset, rcc_utf8_charset, "ISO-8859-2", "CP1250", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"lt", {rcc_default_charset, rcc_utf8_charset, "CP1257", "ISO-8859-4", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"lv", {rcc_default_charset, rcc_utf8_charset, "CP1257", "ISO-8859-4", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"pl", {rcc_default_charset, rcc_utf8_charset, "ISO-8859-2", "CP1250", "IBM852", "macce", "ISO-8859-13", "ISO-8859-16", "baltic", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"sk", {rcc_default_charset, rcc_utf8_charset, "CP1250", "ISO-8859-2", "IBM852", "KEYBCS2", "macce", "KOI-8_CS_2", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"sl", {rcc_default_charset, rcc_utf8_charset, "ISO-8859-2", "CP1250", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"zh", {rcc_default_charset, rcc_utf8_charset, "GB2312", "GBK", "GB18030", "BIG5", NULL},{
    &rcc_default_engine,
    NULL
}},
{NULL}
};

rcc_option_value_name rcc_sn_boolean[] = { "OFF", "ON", NULL };
rcc_option_value_name rcc_sn_learning[] = { "OFF", "ON", "RELEARN", "LEARN", NULL };
rcc_option_value_name rcc_sn_clo[] = { "ALL", "CONFIGURED_AND_AUTO", "CONFIGURED_ONLY", NULL };

rcc_option_description rcc_option_descriptions[RCC_MAX_OPTIONS+1];
rcc_option_description rcc_option_descriptions_embeded[RCC_MAX_OPTIONS+1] = {
    {RCC_OPTION_LEARNING_MODE, 1, { RCC_OPTION_RANGE_TYPE_MENU, 0, 3, 1 }, RCC_OPTION_TYPE_STANDARD,  "LEARNING_MODE", rcc_sn_learning },
    {RCC_OPTION_AUTODETECT_FS_NAMES, 1, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_STANDARD,  "AUTODETECT_FS_NAMES", rcc_sn_boolean},
    {RCC_OPTION_AUTODETECT_FS_TITLES, 1, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_INVISIBLE, "AUTODETECT_FS_TITLES", rcc_sn_boolean},
    {RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, 1, { RCC_OPTION_RANGE_TYPE_MENU, 0, 2, 1}, RCC_OPTION_TYPE_INVISIBLE, "CONFIGURED_LANGUAGES_ONLY", rcc_sn_clo},
    {RCC_OPTION_TRANSLATE, 0, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_STANDARD, "TRANSLATE", rcc_sn_boolean },
    {RCC_OPTION_AUTOENGINE_SET_CURRENT, 0, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_STANDARD, "AUTOENGINE_SET_CURRENT", rcc_sn_boolean },
    {RCC_MAX_OPTIONS}
};

rcc_option_description *rccGetOptionDescription(rcc_option option) {
    unsigned int i;

    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return NULL;

    for (i=0;rcc_option_descriptions[i].option!=RCC_MAX_OPTIONS;i++)
	if (rcc_option_descriptions[i].option == option) return rcc_option_descriptions+i;
    
    return NULL;
}

rcc_option_description *rccGetOptionDescriptionByName(const char *name) {
    unsigned int i;

    if (!name) return NULL;

    for (i=0;rcc_option_descriptions[i].option!=RCC_MAX_OPTIONS;i++)
	if (!strcasecmp(rcc_option_descriptions[i].sn,name)) return rcc_option_descriptions+i;
    
    return NULL;
}

rcc_language_id rccDefaultGetLanguageByName(const char *name) {
    unsigned int i;

    if (!name) return (rcc_language_id)-1;

    for (i=0;rcc_default_languages[i].sn;i++)
	if (!strcasecmp(rcc_default_languages[i].sn, name)) return (rcc_language_id)i;

    return (rcc_language_id)-1;
}

int rccIsUTF8(const char *name) {
    if ((!name)||(strcasecmp(name, "UTF-8")&&strcasecmp(name, "UTF8"))) return 0;
    return 1;
}
