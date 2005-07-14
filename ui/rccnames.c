#include <stdio.h>

#include <librcc.h>

#include "internal.h"

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

rcc_option_value_name rcc_default_option_boolean_names[] = { "Off", "On", NULL };
rcc_option_value_name rcc_default_option_learning_names[] = { "Off", "On", "Relearn", "Learn", NULL };
rcc_option_value_name rcc_default_option_clo_names[] = { "All Languages", "Configured / AutoEngine", "Configured Only", NULL };

rcc_option_name rcc_default_option_names[RCC_MAX_OPTIONS+1] = {
    { RCC_OPTION_LEARNING_MODE, "Recodings Cache", rcc_default_option_learning_names },
    { RCC_OPTION_AUTODETECT_FS_NAMES, "Autodetect File Names",  rcc_default_option_boolean_names },
    { RCC_OPTION_AUTODETECT_FS_TITLES, "Autodetect FS Titles", rcc_default_option_boolean_names },
    { RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, "Enabled Languages", rcc_default_option_clo_names },
    { RCC_MAX_OPTIONS }
};


rcc_language_name *rccUiGetLanguageRccName(rcc_ui_context ctx, const char *lang) {
    const char *res;
    unsigned int i,j;
    rcc_language_name *names;
    
    if (!lang) return NULL;
    
    for (j=0;j<2;j++) {
        if (j) names = rcc_default_language_names;
	else names = ctx?ctx->language_names:NULL;
    
	if (names) {
	    for (i=0;names[i].sn;i++)
		if (!strcmp(lang, names[i].sn)) return names+i;
	}
    }
    
    return NULL;
}

const char *rccUiGetLanguageName(rcc_ui_context ctx, const char *lang) {
    const char *res;
    unsigned int i,j;
    rcc_language_name *names;
    
    names = rccUiGetLanguageRccName(ctx, lang);
    if ((names)&&(names->name)) return names->name;
    
    return lang;
}

rcc_option_name *rccUiGetOptionRccName(rcc_ui_context ctx, rcc_option option) {
    unsigned int i,j;
    rcc_option_name *names;
    
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return NULL;

    for (j=0;j<2;j++) {
	if (j) names = rcc_default_option_names;
	else names = ctx?ctx->option_names:NULL;
	
	if (names) {
	    for (i=0;names[i].option!=RCC_MAX_OPTIONS;i++)
		if (names[i].option == option) return names+i;
	}
    }
    
    return NULL;
}

const char *rccUiGetOptionName(rcc_ui_context ctx, rcc_option option) {
    unsigned int i;
    rcc_option_name *name;
    
    name = rccUiGetOptionRccName(ctx, option);
    if (name) return name->name;

    return NULL;
}

const char *rccUiGetOptionValueName(rcc_ui_context ctx, rcc_option option, rcc_option_value value) {
    unsigned int i;
    rcc_option_name *name;
    
    name = rccUiGetOptionRccName(ctx, option);
    if (name) {
	for (i=0;name->value_names[i];i++)
	    if (i == value) return name->value_names[i];
    }
    
    return NULL;
}
