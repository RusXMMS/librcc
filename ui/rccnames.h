#ifndef _RCC_UI_NAMES_C
#define _RCC_UI_NAMES_H

extern rcc_name rcc_default_language_names[];
extern rcc_name rcc_default_language_names_embeded[];
extern rcc_option_name rcc_default_option_names[];
extern rcc_option_name rcc_default_option_names_embeded[];
extern rcc_ui_page_name rcc_ui_default_page_name;
extern rcc_ui_page_name rcc_ui_default_page_name_embeded;
extern rcc_name *rcc_default_class_names;
extern rcc_name *rcc_default_charset_names;
extern rcc_name *rcc_default_engine_names;

rcc_name *rccUiGetLanguageRccName(rcc_ui_context ctx, const char *lang);
rcc_name *rccUiGetCharsetRccName(rcc_ui_context ctx, const char *charset);
rcc_name *rccUiGetEngineRccName(rcc_ui_context ctx, const char *engine);
rcc_name *rccUiGetClassRccName(rcc_ui_context ctx, const char *cl);

rcc_option_name *rccUiGetOptionRccName(rcc_ui_context ctx, rcc_option option);

#endif /* _RCC_UI_NAMES_H */
