#include <stdio.h>

#include "internal.h"
#include "rccconfig.h"
#include "engine.h"
#include "opt.h"

rcc_language_alias rcc_default_aliases[] = {
    { "cs_SK", "sk" },
    { "ru_UA", "uk" },
    { NULL, NULL}
};

const char rcc_engine_nonconfigured[] = "Default";
const char rcc_option_nonconfigured[] = "DEFAULT";

rcc_engine rcc_default_engine = {
    "Off", NULL, NULL, NULL, {NULL}
};

rcc_engine rcc_russian_engine = {
    "Russian", NULL, NULL, &rccAutoengineRussian, {"CP1251","KOI8-R","UTF-8","IBM866", NULL}
};

rcc_language rcc_default_languages[RCC_MAX_LANGUAGES + 1] = {
{"default", {"Default", NULL}, {
    &rcc_default_engine,
    NULL
}},
{"off", {"Default", NULL}, {
    &rcc_default_engine,
    NULL
}},
{"ru", {"Default","KOI8-R","CP1251","UTF-8","IBM866","MACCYRILLIC","ISO8859-5", NULL}, {
    &rcc_default_engine,
#ifdef RCC_RCD_SUPPORT
    &rcc_russian_engine,
#endif /* RCC_RCD_SUPPORT */
    NULL
}},
{"uk", {"Default","KOI8-U","CP1251","UTF-8","IBM855","MACCYRILLIC","ISO8859-5","CP1125", NULL}, {
    &rcc_default_engine,
#ifdef RCC_RCD_SUPPORT
    &rcc_russian_engine,
#endif /* RCC_RCD_SUPPORT */
    NULL
}},
{"be", {"Default", "UTF-8", "CP1251", "IBM866", "ISO-8859-5", "KOI8-UNI", "maccyr" "IBM855", NULL},{
    &rcc_default_engine,
    NULL
}},
{"bg", {"Default", "UTF-8", "CP1251", "ISO-8859-5", "IBM855", "maccyr", "ECMA-113", NULL},{
    &rcc_default_engine,
    NULL
}},
{"cz", {"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "KEYBCS2", "macce", "KOI-8_CS_2", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"es", {"Default", "UTF-8", "ISO-8859-4", "CP1257", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"hr", {"Default", "UTF-8", "CP1250", "ISO-8859-2", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"hu", {"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"lt", {"Default", "UTF-8", "CP1257", "ISO-8859-4", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"lv", {"Default", "UTF-8", "CP1257", "ISO-8859-4", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"pl", {"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "macce", "ISO-8859-13", "ISO-8859-16", "baltic", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"sk", {"Default", "UTF-8", "CP1250", "ISO-8859-2", "IBM852", "KEYBCS2", "macce", "KOI-8_CS_2", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"sl", {"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"zh", {"Default", "UTF-8", "GB2312", "GBK", "GB18030", "BIG5", NULL},{
    &rcc_default_engine,
    NULL
}},
{NULL}
};

rcc_option_value_name rcc_sn_boolean[] = { "OFF", "ON", NULL };
rcc_option_value_name rcc_sn_learning[] = { "OFF", "ON", "RELEARN", "LEARN", NULL };
rcc_option_value_name rcc_sn_clo[] = { "ALL", "CONFIGURED_AND_AUTO", "CONFIGURED_ONLY", NULL };

rcc_option_description rcc_option_descriptions[] = {
    {RCC_LEARNING_MODE, 0, { RCC_OPTION_RANGE_TYPE_MENU, 0, 3, 1 }, RCC_OPTION_TYPE_STANDARD,  "LEARNING_MODE", rcc_sn_learning },
    {RCC_AUTODETECT_FS_NAMES, 1, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_STANDARD,  "AUTODETECT_FS_NAMES", rcc_sn_boolean},
    {RCC_AUTODETECT_FS_TITLES, 1, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_INVISIBLE, "AUTODETECT_FS_TITLES", rcc_sn_boolean},
    {RCC_CONFIGURED_LANGUAGES_ONLY, 0, { RCC_OPTION_RANGE_TYPE_MENU, 0, 2, 1}, RCC_OPTION_TYPE_INVISIBLE, "CONFIGURED_LANGUAGES_ONLY", rcc_sn_clo},
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
