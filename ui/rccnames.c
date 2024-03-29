/*
  LibRCC - module providing various titles in appropriate language and encoding

  Copyright (C) 2005-2018 Suren A. Chilingaryan <csa@suren.me>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <string.h>

#include <librcc.h>

#include "internal.h"
#include "rccnames.h"

rcc_name rcc_default_language_names[RCC_MAX_LANGUAGES+1];

rcc_name rcc_default_language_names_embeded[RCC_MAX_LANGUAGES+1] = {
{"default", "Autodetect"},
{"off", "Disable"},
{"en", "English" },
{"ru","Russian"},
{"uk","Ukrainian"}, 
{"be","Belarussian"},
{"bg","Bulgarian"},
{"cs","Czech"},
{"et","Estonian"},
{"hr","Croatian"},
{"hu","Hungarian"},
{"lt","Lithuanian"},
{"lv","Latvian"},
{"pl","Polish"},
{"sk","Slovak"},
{"sl","Slovenian"},
{"zh","Chinese"},
{"ko","Korean"},
{"ja","Japanese"},
{NULL, NULL}
};

rcc_option_value_name rcc_default_option_boolean_names[] = { "Off", "On", NULL };
rcc_option_value_name rcc_default_option_learning_names[] = { "Off", "On", "Relearn", "Learn", NULL };
rcc_option_value_name rcc_default_option_clo_names[] = { "All Languages", "Configured / AutoEngine", "Configured Only", NULL };
rcc_option_value_name rcc_default_option_translate_names[] = { "Off", "Transliterate", "Translate to English", "Skip Translation between Related Languages", "Skip Translation from Parrent Languages", "Full", NULL };

rcc_option_name rcc_default_option_names[RCC_MAX_OPTIONS+1];
rcc_option_name rcc_default_option_names_embeded[RCC_MAX_OPTIONS+1] = {
    { RCC_OPTION_LEARNING_MODE, "Recodings Cache", rcc_default_option_learning_names },
    { RCC_OPTION_AUTODETECT_FS_NAMES, "Autodetect File Names",  rcc_default_option_boolean_names },
    { RCC_OPTION_AUTODETECT_FS_TITLES, "Autodetect FS Titles", rcc_default_option_boolean_names },
    { RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, "Enabled Languages", rcc_default_option_clo_names },
    { RCC_OPTION_AUTOENGINE_SET_CURRENT, "AutoEngine Set Current Encoding", rcc_default_option_boolean_names },
    { RCC_OPTION_AUTODETECT_LANGUAGE, "Autodetect Language", rcc_default_option_boolean_names },
    { RCC_OPTION_TRANSLATE, "Translate Text", rcc_default_option_translate_names },
    { RCC_OPTION_TIMEOUT, "Recoding Timeout (us)", NULL },
    { RCC_MAX_OPTIONS }
};

rcc_ui_page_name rcc_ui_default_page_name;
rcc_ui_page_name rcc_ui_default_page_name_embeded = {
    "Encodings",
    { "Language", "Language" },
    { "Encodings" },
    { "AutoDetection", "Engine" }
};


rcc_name *rcc_default_class_names = NULL;
rcc_name *rcc_default_charset_names = NULL;
rcc_name *rcc_default_engine_names = NULL;

typedef enum rcc_name_type_t {
    RCC_NAME_TYPE_CLASS = 0,
    RCC_NAME_TYPE_CHARSET,
    RCC_NAME_TYPE_ENGINE,
    RCC_NAME_TYPE_LANGUAGE,
    RCC_NAME_TYPE_MAX
} rcc_name_type;

rcc_name *rccUiGetRccName(rcc_ui_context ctx, const char *findname, unsigned char type) {
    unsigned int i,j;
    rcc_name *names[2], *nm;
    
    if (!findname) return NULL;
    
    switch (type) {
	case RCC_NAME_TYPE_LANGUAGE:
	    names[0] = ctx?ctx->language_names:NULL;
	    names[1] = rcc_default_language_names;
	break;
	case RCC_NAME_TYPE_CHARSET:
	    names[0] = ctx?ctx->charset_names:NULL;
	    names[1] = rcc_default_charset_names;
	break;
	case RCC_NAME_TYPE_ENGINE:
	    names[0] = ctx?ctx->engine_names:NULL;
	    names[1] = rcc_default_engine_names;
	break;
	case RCC_NAME_TYPE_CLASS:
	    if (ctx->class_names) return NULL;
	    
	    names[0] = NULL;
	    names[1] = rcc_default_class_names;
	break;	
	default:
	    return NULL;
    }
	
    
    for (j=0;j<2;j++) {
	nm = names[j];
	if (nm) {
	    for (i=0;nm[i].sn;i++)
		if (!strcasecmp(findname, nm[i].sn)) return nm+i;
	}
    }
    
    return NULL;
}

rcc_name *rccUiGetLanguageRccName(rcc_ui_context ctx, const char *lang) {
    return rccUiGetRccName(ctx, lang, RCC_NAME_TYPE_LANGUAGE);
}

const char *rccUiGetLanguageName(rcc_ui_context ctx, rcc_language_id language_id) {
    const char *lang;
    rcc_name *names;
    
    lang = rccGetLanguageName(ctx->rccctx, language_id);
    
    names = rccUiGetLanguageRccName(ctx, lang);
    if ((names)&&(names->name)) return names->name;
    
    return lang;
}

rcc_name *rccUiGetCharsetRccName(rcc_ui_context ctx, const char *charset) {
    return rccUiGetRccName(ctx, charset, RCC_NAME_TYPE_CHARSET);
}

const char *rccUiGetCharsetName(rcc_ui_context ctx, rcc_language_id language_id, rcc_class_id class_id, rcc_charset_id charset_id) {
    rcc_language_config config;
    const char *charset;
    rcc_name *names;

    config = rccGetConfig(ctx->rccctx, language_id);
    charset = rccConfigGetClassCharsetName(config, class_id, charset_id);
    
    names = rccUiGetCharsetRccName(ctx, charset);
    if ((names)&&(names->name)) return names->name;
    
    return charset;
}

rcc_name *rccUiGetEngineRccName(rcc_ui_context ctx, const char *engine) {
    return rccUiGetRccName(ctx, engine, RCC_NAME_TYPE_ENGINE);
}

const char *rccUiGetEngineName(rcc_ui_context ctx, rcc_language_id language_id, rcc_engine_id engine_id) {
    rcc_language_config config;
    const char *engine;
    rcc_name *names;
    
    config = rccGetConfig(ctx->rccctx, language_id);
    engine = rccConfigGetEngineName(config, engine_id);

    names = rccUiGetEngineRccName(ctx, engine);
    if ((names)&&(names->name)) return names->name;
    
    return engine;
}

rcc_name *rccUiGetClassRccName(rcc_ui_context ctx, const char *cl) {
    return rccUiGetRccName(ctx, cl, RCC_NAME_TYPE_CLASS);
}

const char *rccUiGetClassName(rcc_ui_context ctx, rcc_class_id class_id) {
    const char *cl;
    rcc_name *names;
    
    cl = rccGetClassName(ctx->rccctx, class_id);
    
    names = rccUiGetClassRccName(ctx, cl);
    if ((names)&&(names->name)) return names->name;
    
    return rccGetClassFullName(ctx->rccctx, (rcc_class_id)class_id);
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

rcc_ui_page_name *rccUiGetDefaultPageName() {
    return &rcc_ui_default_page_name;
}
