#include <stdio.h>
#include <strings.h>

#include "internal.h"
#include "rccconfig.h"
#include "engine.h"
#include "opt.h"

#define RCC_DEFAULT_RECODING_TIMEOUT 500000

rcc_language_alias rcc_default_aliases[RCC_MAX_ALIASES + 1];
rcc_language_alias rcc_default_aliases_embeded[RCC_MAX_ALIASES + 1] = {
/*    { "cs_SK", "sk" },
    { "ru_UA", "uk" },*/
    { NULL, NULL }
};

rcc_language_relation rcc_default_relations[RCC_MAX_RELATIONS + 1];
rcc_language_relation rcc_default_relations_embeded[RCC_MAX_RELATIONS + 1] = {
    { "uk", "ru" },
    { "be", "ru" },
    { NULL, NULL }
};

const char rcc_default_unicode_charsets[] = "unicode";
const char rcc_default_nonunicode_charsets[] = "nonunicode";
rcc_charset rcc_default_disabled_id3_charsets[] = { rcc_default_unicode_charsets, NULL };
rcc_charset rcc_default_additional_id3v2_charsets[] = { "UTF-8", "UTF-16", "UTF-16BE", NULL };

const char rcc_default_all[] = "all";
const char rcc_default_language_sn[] = "default";
const char rcc_disabled_language_sn[] = "Off";
const char rcc_english_language_sn[] = "en";
const char rcc_russian_language_sn[] = "ru";
const char rcc_ukrainian_language_sn[] = "uk";
const char rcc_disabled_engine_sn[] = "Off";
const char rcc_default_charset[] = "Default";

const char rcc_utf8_charset[] = "UTF-8";
const char rcc_engine_nonconfigured[] = "Default";
const char rcc_option_nonconfigured[] = "DEFAULT";

rcc_engine rcc_default_engine = {
    rcc_disabled_engine_sn, NULL, NULL, NULL, {NULL}
};

rcc_engine rcc_russian_engine = {
    "LibRCD", NULL, NULL, &rccAutoengineRussian, {"CP1251","KOI8-R","UTF-8","IBM866", "ISO8859-1", NULL}
};

rcc_engine rcc_ukrainian_engine = {
    "LibRCD", NULL, NULL, &rccAutoengineRussian, {"CP1251","KOI8-U","UTF-8","IBM865", "ISO8859-1", NULL}
};

rcc_engine rcc_belarussian_engine = {
    "LibRCD", NULL, NULL, &rccAutoengineRussian, {"CP1251","ISO-IR-111","UTF-8","IBM865", "ISO8859-1", NULL}
};

rcc_language rcc_default_languages[RCC_MAX_LANGUAGES + 1];

rcc_language rcc_default_languages_embeded[RCC_MAX_LANGUAGES + 1] = {
{rcc_default_language_sn, {rcc_default_charset, NULL}, {
    &rcc_default_engine,
    NULL
}},
{rcc_disabled_language_sn, {rcc_default_charset, NULL}, {
    &rcc_default_engine,
    NULL
}},
{rcc_english_language_sn, {rcc_default_charset, rcc_utf8_charset, "ISO8859-1", NULL}, {
    &rcc_default_engine,
    NULL
}},
{rcc_russian_language_sn, {rcc_default_charset,"KOI8-R","CP1251",rcc_utf8_charset,"IBM866","MACCYRILLIC","ISO8859-5", NULL}, {
    &rcc_default_engine,
#ifdef RCC_RCD_SUPPORT
    &rcc_russian_engine,
#endif /* RCC_RCD_SUPPORT */
    NULL
}},
{rcc_ukrainian_language_sn, {rcc_default_charset,"KOI8-U","CP1251",rcc_utf8_charset,"IBM855","MACCYRILLIC","ISO8859-5","CP1125", NULL}, {
    &rcc_default_engine,
#ifdef RCC_RCD_SUPPORT
    &rcc_ukrainian_engine,
#endif /* RCC_RCD_SUPPORT */
    NULL
}},
{"be", {rcc_default_charset, rcc_utf8_charset, "CP1251", "IBM866", "ISO-8859-5", "ISO-IR-111", "ISO-IR-111", "MACCYRILLIC" "IBM855", NULL},{
    &rcc_default_engine,
#ifdef RCC_RCD_SUPPORT
    &rcc_ukrainian_engine,
#endif /* RCC_RCD_SUPPORT */
    NULL
}},
/*{"bg", {rcc_default_charset, rcc_utf8_charset, "CP1251", "ISO-8859-5", "IBM855", "maccyr", "ECMA-113", NULL},{
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
}},*/
{NULL}
};
rcc_option_value_name rcc_sn_boolean[] = { "OFF", "ON", NULL };
rcc_option_value_name rcc_sn_learning[] = { "OFF", "ON", "RELEARN", "LEARN", NULL };
rcc_option_value_name rcc_sn_clo[] = { "ALL", "CONFIGURED_AND_AUTO", "CONFIGURED_ONLY", NULL };
rcc_option_value_name rcc_sn_translate[] = { "OFF", "TRANSLITERATE", "TO_ENGLISH", "SKIP_RELATED", "SKIP_PARENT", "FULL", NULL };

rcc_option_description rcc_option_descriptions[RCC_MAX_OPTIONS+1];
rcc_option_description rcc_option_descriptions_embeded[RCC_MAX_OPTIONS+1] = {
#ifdef HAVE_DB_H
    {RCC_OPTION_LEARNING_MODE, 1, { RCC_OPTION_RANGE_TYPE_MENU, 0, 3, 1 }, RCC_OPTION_TYPE_STANDARD,  "LEARNING_MODE", rcc_sn_learning },
#else
    {RCC_OPTION_LEARNING_MODE, 1, { RCC_OPTION_RANGE_TYPE_MENU, 0, 3, 1 }, RCC_OPTION_TYPE_INVISIBLE,  "LEARNING_MODE", rcc_sn_learning },
#endif /* HAVE_DB_H */
    {RCC_OPTION_AUTODETECT_FS_NAMES, 1, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_STANDARD,  "AUTODETECT_FS_NAMES", rcc_sn_boolean},
    {RCC_OPTION_AUTODETECT_FS_TITLES, 1, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_INVISIBLE, "AUTODETECT_FS_TITLES", rcc_sn_boolean},
    {RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, 1, { RCC_OPTION_RANGE_TYPE_MENU, 0, 2, 1}, RCC_OPTION_TYPE_INVISIBLE, "CONFIGURED_LANGUAGES_ONLY", rcc_sn_clo},
#ifdef HAVE_ASPELL
    {RCC_OPTION_AUTODETECT_LANGUAGE, 0, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_STANDARD, "AUTODETECT_LANGUAGE", rcc_sn_boolean},
#else
    {RCC_OPTION_AUTODETECT_LANGUAGE, 0, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_INVISIBLE, "AUTODETECT_LANGUAGE", rcc_sn_boolean},
#endif 
#ifdef HAVE_LIBTRANSLATE
    {RCC_OPTION_TRANSLATE, 0, { RCC_OPTION_RANGE_TYPE_MENU, 0, 3, 1}, RCC_OPTION_TYPE_STANDARD, "TRANSLATE", rcc_sn_translate },
#else
    {RCC_OPTION_TRANSLATE, 0, { RCC_OPTION_RANGE_TYPE_MENU, 0, 3, 1}, RCC_OPTION_TYPE_INVISIBLE, "TRANSLATE", rcc_sn_translate },
#endif /* HAVE_LIBTRANSLATE */
    {RCC_OPTION_AUTOENGINE_SET_CURRENT, 0, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0}, RCC_OPTION_TYPE_STANDARD, "AUTOENGINE_SET_CURRENT", rcc_sn_boolean },
#ifdef HAVE_LIBTRANSLATE
    {RCC_OPTION_TIMEOUT, RCC_DEFAULT_RECODING_TIMEOUT, { RCC_OPTION_RANGE_TYPE_RANGE, 0, 5000000, 50000}, RCC_OPTION_TYPE_STANDARD, "TIMEOUT", NULL },
#else
    {RCC_OPTION_TIMEOUT, RCC_DEFAULT_RECODING_TIMEOUT, { RCC_OPTION_RANGE_TYPE_RANGE, 0, 5000000, 50000}, RCC_OPTION_TYPE_INVISIBLE, "TIMEOUT", NULL },
#endif /* HAVE_LIBTRANSLATE */
    {RCC_OPTION_OFFLINE, 0, { RCC_OPTION_RANGE_TYPE_BOOLEAN, 0, 0, 0 }, RCC_OPTION_TYPE_INVISIBLE, "OFFLINE_PROCESSING", rcc_sn_boolean},
    {RCC_MAX_OPTIONS}
};

rcc_option_description *rccGetOptionDescription(rcc_option option) {
    unsigned int i;

    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return NULL;

    for (i=0;rcc_option_descriptions[i].option!=RCC_MAX_OPTIONS;i++)
	if (rcc_option_descriptions[i].option == option) 
	    return rcc_option_descriptions+i;
    
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

int rccIsUnicode(const char *name) {
    if ((!name)||(strncasecmp(name, "UTF",3)&&strncasecmp(name, "UCS",3))) return 0;
    return 1;
}

unsigned int rccDefaultDropLanguageRelations(const char *lang) {
    unsigned long i, j;
    for (i=0,j=0;rcc_default_relations[i].lang;i++) {
	if (strcasecmp(lang, rcc_default_relations[i].lang)) {
	    if (j<i) {
		rcc_default_relations[j].lang = rcc_default_relations[i].lang;
		rcc_default_relations[j++].parent = rcc_default_relations[i].parent;
	    } else j++;
	}
    }
    rcc_default_relations[j].lang = NULL;
    rcc_default_relations[j].parent = NULL;
    return j;
}
