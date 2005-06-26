#include <stdio.h>
#include <librcd.h>

#include "config.h"

static rcc_charset_id rcc_autoengine_russian(rcc_engine_context ctx, char *buf, int len) {
    return (int)get_russian_charset(buf,len);
}

rcc_language_alias rcc_default_aliases[] = {
    { "cs_SK", "sk" },
    { "ru_UA", "uk" },
    { NULL, NULL}
};

const char rcc_engine_nonconfigured[] = "NonConfigured";

rcc_engine rcc_default_engine = {
    "Off", NULL, NULL, NULL, {NULL}
};

rcc_engine rcc_russian_engine = {
    "Russian", NULL, NULL, &rcc_autoengine_russian, {"CP1251","KOI8-R","UTF-8","IBM866", NULL}
};

rcc_language rcc_default_languages[] = {
{"default", "Autodetect", {"Default", NULL}, {
    &rcc_default_engine,
    NULL
}},
{"off", "Dissable", {"Default", NULL}, {
    &rcc_default_engine,
    NULL
}},
{"ru","Russian", {"Default","KOI8-R","CP1251","UTF-8","IBM866","MACCYRILLIC","ISO8859-5", NULL}, {
    &rcc_russian_engine,
    &rcc_default_engine,
    NULL
}},
{"uk","Ukrainian", {"Default","KOI8-U","CP1251","UTF-8","IBM855","MACCYRILLIC","ISO8859-5","CP1125", NULL}, {
    &rcc_russian_engine,
    &rcc_default_engine,
    NULL
}},
{"be","Belarussian",{"Default", "UTF-8", "CP1251", "IBM866", "ISO-8859-5", "KOI8-UNI", "maccyr" "IBM855", NULL},{
    &rcc_default_engine,
    NULL
}},
{"bg","Bulgarian",{"Default", "UTF-8", "CP1251", "ISO-8859-5", "IBM855", "maccyr", "ECMA-113", NULL},{
    &rcc_default_engine,
    NULL
}},
{"cz","Czech",{"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "KEYBCS2", "macce", "KOI-8_CS_2", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"es","Estonian",{"Default", "UTF-8", "ISO-8859-4", "CP1257", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"hr","Croatian",{"Default", "UTF-8", "CP1250", "ISO-8859-2", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"hu","Hungarian",{"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"lt","Lithuanian",{"Default", "UTF-8", "CP1257", "ISO-8859-4", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"lv","Latvian",{"Default", "UTF-8", "CP1257", "ISO-8859-4", "IBM775", "ISO-8859-13", "macce", "baltic", NULL},{
    &rcc_default_engine,
    NULL
}},
{"pl","Polish",{"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "macce", "ISO-8859-13", "ISO-8859-16", "baltic", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"sk","Slovak",{"Default", "UTF-8", "CP1250", "ISO-8859-2", "IBM852", "KEYBCS2", "macce", "KOI-8_CS_2", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"sl","Slovenian",{"Default", "UTF-8", "ISO-8859-2", "CP1250", "IBM852", "macce", "CORK", NULL},{
    &rcc_default_engine,
    NULL
}},
{"zh","Chinese",{"Default", "UTF-8", "GB2312", "GBK", "GB18030", "BIG5", NULL},{
    &rcc_default_engine,
    NULL
}},
{NULL}
};

/*
const charset_list_t charset_list_default = { "Default",  NULL };
charset_t *charset_list=(charset_t*)charset_list_default;
#define autocharset_list_ni_default 1
autocharset_list_t autocharset_list_default = {
    {"Off", NULL, {NULL}}
};

int autocharset_list_ni=autocharset_list_ni_default;
autocharset_t *autocharset_list=(autocharset_t*)autocharset_list_default;
*/