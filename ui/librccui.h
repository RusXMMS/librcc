/*
  LibRCC - public interface

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

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

const char *rccUiGetLanguageName(rcc_ui_context ctx, rcc_language_id language_id);
const char *rccUiGetClassName(rcc_ui_context ctx, rcc_class_id class_id);
const char *rccUiGetCharsetName(rcc_ui_context ctx, rcc_language_id language_id, rcc_class_id class_id, rcc_charset_id charset_id);
const char *rccUiGetEngineName(rcc_ui_context ctx, rcc_language_id language_id, rcc_engine_id engine_id);
const char *rccUiGetOptionName(rcc_ui_context ctx, rcc_option option);
const char *rccUiGetOptionValueName(rcc_ui_context ctx, rcc_option option, rcc_option_value value);

/*******************************************************************************
************************************ API ***************************************
*******************************************************************************/

typedef void *rcc_ui_widget;
typedef void *rcc_ui_box;
typedef void *rcc_ui_frame;
typedef void *rcc_ui_page;

/**
  * Init RCC User Interface Library. This function should be called prior to any other UI functions 
  */
int rccUiInit();
/**
  * Free resources used by UI library. 
  *
  * @return non-zero value in the case of error.
  */
void rccUiFree();

/**
  * Create UI context and attach it to RCC working context. It isn't permited to attach to single
  * working context several UI's.
  * 
  * @param rccctx is pointer on the RCC working context.
  * @return pointer on newly created UI context or NULL in the case of errors.
  */
rcc_ui_context rccUiCreateContext(rcc_context rccctx);
/**
  * Destroy UI context and free all memory used. 
  *
  * @param ctx is UI context to be destroyed.
  */
void rccUiFreeContext(rcc_ui_context ctx);

int rccUiHideOption(rcc_ui_context ctx, rcc_option option);
int rccUiUnHideOption(rcc_ui_context ctx, rcc_option option);

/**
  * Restores current language UI menu values from #rcc_context.
  *
  * @param ctx is UI context.
  */
int rccUiRestoreLanguage(rcc_ui_context ctx);
/**
  * Restores complete UI configuration from #rcc_context.
  *
  * @param ctx is UI context.
  */
int rccUiRestore(rcc_ui_context ctx);
/**
  * Updates #rcc_context with user adjusted options from UI configuration.
  *
  * @param ctx is UI context.
  */
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

/**
  * Get UI option page which can be integrated in application GUI. It is up on the caller
  * to use and destroy returned page.
  *
  * @param ctx is UI context.
  * @param name provides titles which should be used on the page or NULL to use defaults.
  * @result is created UI page or NULL in the case of errors.
  */
rcc_ui_page rccUiGetPage(rcc_ui_context ctx, rcc_ui_page_name *name);

#ifdef __cplusplus
}
#endif

#endif /* _LIBRCC_UI_H */
