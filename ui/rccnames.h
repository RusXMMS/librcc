#ifndef _RCC_UI_NAMES_C
#define _RCC_UI_NAMES_H

extern rcc_language_name rcc_default_language_names[];
extern rcc_option_name rcc_default_option_names[];

rcc_language_name *rccUiGetLanguageRccName(rcc_ui_context ctx, const char *lang);
const char *rccUiGetLanguageName(rcc_ui_context ctx, const char *lang);
rcc_option_name *rccUiGetOptionRccName(rcc_ui_context ctx, rcc_option option);
const char *rccUiGetOptionName(rcc_ui_context ctx, rcc_option option);
const char *rccUiGetOptionValueName(rcc_ui_context ctx, rcc_option option, rcc_option_value value);

#endif /* _RCC_UI_NAMES_H */
