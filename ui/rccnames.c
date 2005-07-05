#include <stdio.h>

#include "internal.h"

const char *rccUiGetLanguageName(rcc_ui_context ctx, const char *lang) {
    const char *res;
    unsigned int i;
    rcc_language_name *names;
    
    if ((!ctx)||(!lang)) return NULL;
    
    names = ctx->language_names;
    
    if (names) {
	for (i=0;names[i].sn;i++)
	    if (!strcmp(lang, names[i].sn)) return names[i].name;
    }


    res = rccGetLanguageFullName(lang);
    if (res) return res;
    
    return lang;
}

const char *rccUiGetOptionName(rcc_ui_context ctx, rcc_option option) {
    const char *res;
    unsigned int i;
    rcc_option_name *names;
    
    if ((!ctx)||(option<0)||(option>RCC_MAX_OPTIONS)) return NULL;

    names = ctx->option_names;

    if (names) {
	for (i=0;((i<RCC_MAX_OPTIONS)&&(names[i].option!=RCC_MAX_OPTIONS));i++)
	    if (names[i].option == option) return names[i].name;
    }

    res = rccGetOptionFullName(option);
    if (res) return res;
    
    return "";
}
