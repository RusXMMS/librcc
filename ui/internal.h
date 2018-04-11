/*
  LibRCC UI - internal data structures

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

#ifndef _RCC_UI_INTERNAL_H
#define _RCC_UI_INTERNAL_H

#define RCC_UI_MAX_STRING_CHARS 1024

#include <librcc.h>

#include "rccmenu.h"

typedef unsigned int rcc_ui_id;

typedef void *rcc_ui_frame_internal;

enum rcc_ui_frame_type_t {
    RCC_UI_FRAME_LANGUAGE = 0,
    RCC_UI_FRAME_CHARSETS,
    RCC_UI_FRAME_ENGINE,
    RCC_UI_FRAME_MAX
};
typedef enum rcc_ui_frame_type_t rcc_ui_frame_type;

struct rcc_ui_frame_context_t {
    rcc_ui_frame frame;
    
    rcc_ui_context uictx;
    rcc_ui_frame_type type;
    
    rcc_ui_frame_internal internal;
};
typedef struct rcc_ui_frame_context_t rcc_ui_frame_context_s;
typedef struct rcc_ui_frame_context_t *rcc_ui_frame_context;


typedef void *rcc_ui_internal;

struct rcc_ui_context_t {
    rcc_context rccctx;

    int n_classes;    
    rcc_ui_menu_context language;
    rcc_ui_menu_context engine;
    rcc_ui_menu_context *charsets;
    rcc_ui_menu_context *options;

    rcc_ui_internal internal;
    
    rcc_name *language_names;
    rcc_name *charset_names;
    rcc_name *engine_names;
    rcc_option_name *option_names;
    unsigned char class_names;
    
    rcc_ui_frame_context language_frame;
    rcc_ui_frame_context charset_frame;
    rcc_ui_frame_context engine_frame;
    
    rcc_ui_page page;
};
typedef struct rcc_ui_context_t rcc_ui_contexts_s;


rcc_ui_internal rccUiCreateInternal(rcc_ui_context ctx);
void rccUiFreeInternal(rcc_ui_context ctx);

rcc_ui_widget rccUiMenuCreateWidget(rcc_ui_menu_context ctx);
void rccUiMenuFreeWidget(rcc_ui_menu_context ctx);
int rccUiMenuConfigureWidget(rcc_ui_menu_context ctx);

rcc_ui_id rccUiMenuGet(rcc_ui_menu_context ctx);
int rccUiMenuSet(rcc_ui_menu_context ctx, rcc_ui_id id);

rcc_ui_box rccUiBoxCreate(rcc_ui_menu_context ctx, const char *title);

rcc_ui_frame rccUiFrameCreate(rcc_ui_frame_context ctx, const char *title);
void rccUiFrameFree(rcc_ui_frame_context ctx);
int rccUiFrameAdd(rcc_ui_frame_context ctx, rcc_ui_box box);

rcc_ui_page rccUiPageCreate(rcc_ui_context ctx, const char *title);
int rccUiPageAdd(rcc_ui_page page, rcc_ui_frame frame);

#endif /* _RCC_UI_INTERNAL_H */
