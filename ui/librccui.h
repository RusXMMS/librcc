#ifndef _LIBRCC_UI_H
#define _LIBRCC_UI_H

#include <librcc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rcc_ui_context_t *rcc_ui_context;

/*******************************************************************************
************************************ Names *************************************
*******************************************************************************/

struct rcc_name_t {
    const char *sn;
    const char *name;
};
typedef struct rcc_name_t rcc_name;

typedef const char *rcc_option_value_name;
typedef rcc_option_value_name *rcc_option_value_names;

struct rcc_option_name_t {
    rcc_option option;
    const char *name;
    rcc_option_value_names value_names;
};
typedef struct rcc_option_name_t rcc_option_name;

int rccUiSetLanguageNames(rcc_ui_context ctx, rcc_name *names);
int rccUiSetCharsetNames(rcc_ui_context ctx, rcc_name *names);
int rccUiSetEngineNames(rcc_ui_context ctx, rcc_name *names);

int rccUiSetOptionNames(rcc_ui_context ctx, rcc_option_name *names);
int rccUiSetClassNames(rcc_ui_context ctx);

/* page & boxes */
typedef const char *rcc_ui_box_name;
struct rcc_ui_language_frame_name_t {
    const char *title;
    rcc_ui_box_name language;
};
typedef struct rcc_ui_language_frame_name_t rcc_ui_language_frame_name;
struct rcc_ui_charset_frame_name_t {
    const char *title;
};
typedef struct rcc_ui_charset_frame_name_t rcc_ui_charset_frame_name;
struct rcc_ui_engine_frame_name_t {
    const char *title;
    rcc_ui_box_name engine;
};
typedef struct rcc_ui_engine_frame_name_t rcc_ui_engine_frame_name;
struct rcc_ui_page_name_t {
    const char *title;
    rcc_ui_language_frame_name language;
    rcc_ui_charset_frame_name charset;
    rcc_ui_engine_frame_name engine;
};
typedef struct rcc_ui_page_name_t rcc_ui_page_name;

rcc_ui_page_name *rccUiGetDefaultPageName();


/*******************************************************************************
************************************ API ***************************************
*******************************************************************************/

typedef void *rcc_ui_widget;
typedef void *rcc_ui_box;
typedef void *rcc_ui_frame;
typedef void *rcc_ui_page;

int rccUiInit();
void rccUiFree();

rcc_ui_context rccUiCreateContext(rcc_context rccctx);
void rccUiFreeContext(rcc_ui_context ctx);

int rccUiRestoreLanguage(rcc_ui_context ctx);
int rccUiRestore(rcc_ui_context ctx);
int rccUiUpdate(rcc_ui_context ctx);

rcc_ui_widget rccUiGetLanguageMenu(rcc_ui_context ctx);
rcc_ui_widget rccUiGetCharsetMenu(rcc_ui_context ctx, rcc_class_id id);
rcc_ui_widget rccUiGetEngineMenu(rcc_ui_context ctx);
rcc_ui_widget rccUiGetOptionMenu(rcc_ui_context ctx, rcc_option option);

rcc_ui_box rccUiGetLanguageBox(rcc_ui_context ctx, const char *title);
rcc_ui_box rccUiGetCharsetBox(rcc_ui_context ctx, rcc_class_id id, const char *title);
rcc_ui_box rccUiGetEngineBox(rcc_ui_context ctx, const char *title);
rcc_ui_box rccUiGetOptionBox(rcc_ui_context ctx, rcc_option option, const char *title);

rcc_ui_frame rccUiGetLanguageFrame(rcc_ui_context ctx, rcc_ui_language_frame_name *name);
rcc_ui_frame rccUiGetCharsetsFrame(rcc_ui_context ctx, rcc_ui_charset_frame_name *name);
rcc_ui_frame rccUiGetEngineFrame(rcc_ui_context ctx, rcc_ui_engine_frame_name *name);

rcc_ui_page rccUiGetPage(rcc_ui_context ctx, rcc_ui_page_name *name);

#ifdef __cplusplus
}
#endif

#endif /* _LIBRCC_UI_H */
