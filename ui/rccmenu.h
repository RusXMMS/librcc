#ifndef _RCC_UI_MENU_H
#define _RCC_UI_MENU_H

#include "librccui.h"

typedef void *rcc_ui_menu_internal;

enum rcc_ui_menu_type_t {
    RCC_UI_MENU_LANGUAGE = 0,
    RCC_UI_MENU_CHARSET,
    RCC_UI_MENU_ENGINE,
    RCC_UI_MENU_OPTION,
    RCC_UI_MENU_MAX
};
typedef enum rcc_ui_menu_type_t rcc_ui_menu_type;

struct rcc_ui_menu_context_t {
    rcc_ui_widget widget;
    rcc_ui_box box;
    
    rcc_ui_context uictx;
    rcc_ui_menu_type type;
    
    rcc_ui_menu_internal internal;
};
typedef struct rcc_ui_menu_context_t rcc_ui_menu_context_s;
typedef struct rcc_ui_menu_context_t *rcc_ui_menu_context;

struct rcc_ui_charset_menu_context_t {
    rcc_ui_menu_context_s ui_menu;
    rcc_class_id id;
};
typedef struct rcc_ui_charset_menu_context_t rcc_ui_charset_menu_context_s;
typedef struct rcc_ui_charset_menu_context_t *rcc_ui_charset_menu_context;

struct rcc_ui_option_menu_context_t {
    rcc_ui_menu_context_s ui_menu;
    rcc_option id;
    rcc_option_type type;
    rcc_option_range *range;
};
typedef struct rcc_ui_option_menu_context_t rcc_ui_option_menu_context_s;
typedef struct rcc_ui_option_menu_context_t *rcc_ui_option_menu_context;

rcc_ui_menu_context rccUiMenuCreateContext(rcc_ui_menu_type type, rcc_ui_context uictx);
rcc_ui_menu_context rccUiCharsetMenuCreateContext(rcc_ui_menu_type type, rcc_charset_id id, rcc_ui_context uictx);
rcc_ui_menu_context rccUiOptionMenuCreateContext(rcc_ui_menu_type type, rcc_option id, rcc_option_type otype, rcc_option_range *range, rcc_ui_context uictx);
void rccUiMenuFreeContext(rcc_ui_menu_context ctx);

rcc_class_id rccUiMenuGetClassId(rcc_ui_menu_context ctx);
rcc_option rccUiMenuGetOption(rcc_ui_menu_context ctx);
rcc_option_type rccUiMenuGetType(rcc_ui_menu_context ctx);
rcc_option_range_type rccUiMenuGetRangeType(rcc_ui_menu_context ctx);

#endif /* _RCC_UI_MENU_H */
