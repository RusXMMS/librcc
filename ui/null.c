/*
  LibRCC UI - dummy library

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
#include <librcc.h>

#include "internal.h"

rcc_ui_internal rccUiCreateInternal(rcc_ui_context ctx) {
    return NULL;
}

void rccUiFreeInternal(rcc_ui_context ctx) {
}

rcc_ui_widget rccUiMenuCreateWidget(rcc_ui_menu_context ctx) {
    return NULL;
}

void rccUiMenuFreeWidget(rcc_ui_menu_context ctx) {
}

rcc_ui_id rccUiMenuGet(rcc_ui_menu_context ctx) {
    return 0;
}

int rccUiMenuSet(rcc_ui_menu_context ctx, rcc_ui_id id) {
    return 0;
}


int rccUiMenuConfigureWidget(rcc_ui_menu_context ctx) {
    ctx->widget = NULL;
    return 0;
}


rcc_ui_box rccUiBoxCreate(rcc_ui_menu_context ctx, const char *title) {
    return (rcc_ui_box)NULL;
}

rcc_ui_frame rccUiFrameCreate(rcc_ui_frame_context ctx, const char *title) {
    return (rcc_ui_frame)NULL;
}

void rccUiFrameFree(rcc_ui_frame_context ctx) {
}


int rccUiFrameAdd(rcc_ui_frame_context ctx, rcc_ui_box box) {
    return 0;
}

rcc_ui_page rccUiPageCreate(rcc_ui_context ctx, const char *title) {
    return (rcc_ui_page)NULL;
}

int rccUiPageAdd(rcc_ui_page page, rcc_ui_frame frame) {
    return 0;
}
