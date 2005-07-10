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

const char rcc_engine_nonconfigured[] = "default";
const char rcc_option_nonconfigured[] = "default";

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

rcc_language_name rcc_default_language_names[RCC_MAX_LANGUAGES+1] = {
{"default", "Autodetect"},
{"off", "Dissable"},
{"ru","Russian"},
{"uk","Ukrainian"}, 
{"be","Belarussian"},
{"bg","Bulgarian"},
{"cz","Czech"},
{"es","Estonian"},
{"hr","Croatian"},
{"hu","Hungarian"},
{"lt","Lithuanian"},
{"lv","Latvian"},
{"pl","Polish"},
{"sk","Slovak"},
{"sl","Slovenian"},
{"zh","Chinese"},
{NULL, NULL}
};

rcc_option_description rcc_default_option_descriptions[] = {
    {RCC_LEARNING_MODE, "LEARNING_MODE", RCC_OPTION_TYPE_MENU, 0, 3},
    {RCC_AUTODETECT_FS_NAMES, "AUTODETECT_FS_NAMES", RCC_OPTION_TYPE_BOOLEAN, 0, 0},
    {RCC_AUTODETECT_FS_TITLES, "AUTODETECT_FS_TITLES", RCC_OPTION_TYPE_BOOLEAN, 0, 0},
    {RCC_CONFIGURED_LANGUAGES_ONLY, "CONFIGURED_LANGUAGES_ONLY", RCC_OPTION_MENU, 0, 2},
    {RCC_MAX_OPTIONS, NULL}
};


rcc_option_name rcc_default_option_names[] = {
    { RCC_LEARNING_MODE, "Learning Mode", },
    { RCC_AUTODETECT_FS_NAMES, "Autodetect File Names",  NULL},
    { RCC_AUTODETECT_FS_TITLES, NULL, NULL},
    { RCC_CONFIGURED_LANGUAGES_ONLY, NULL, NULL},
    { RCC_MAX_OPTIONS, NULL, NULL}
};

rcc_option_defval rcc_default_option_values[] = {
    { RCC_LEARNING_MODE, 0 },
    { RCC_AUTODETECT_FS_TITLES, 1},
    { RCC_AUTODETECT_FS_NAMES, 1},
    { RCC_CONFIGURED_LANGUAGES_ONLY, 1},
    { RCC_MAX_OPTIONS, 0}
};

const char *rccGetLanguageFullName(const char *lang) {
    unsigned int i;
    rcc_language_name *names;
    
    if (!lang) return NULL;
    
    names = rcc_default_language_names;

    for (i=0;names[i].sn;i++)
	if (!strcmp(lang, names[i].sn)) return names[i].name;
    
    return NULL;
}

const char *rccGetOptionName(rcc_option option) {
    unsigned int i;
    rcc_option_name *names;
    
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return NULL;

    names = rcc_default_option_names;

    for (i=0;names[i].option!=RCC_MAX_OPTIONS;i++)
	if (names[i].option == option) return names[i].sn;
    
    return NULL;
}

const char *rccGetOptionFullName(rcc_option option) {
    unsigned int i;
    rcc_option_name *names;
    
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return NULL;

    names = rcc_default_option_names;

    for (i=0;names[i].option!=RCC_MAX_OPTIONS;i++)
	if (names[i].option == option) return names[i].name;
    
    return NULL;
}

rcc_option_value rccGetOptionDefaultValue(rcc_option option) {
    unsigned int i;
    rcc_option_defval *values;
    
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return 0;

    values = rcc_default_option_values;

    for (i=0;values[i].option!=RCC_MAX_OPTIONS;i++)
	if (values[i].option == option) return values[i].value;
    
    return 0;
}