/*
  LibRCC UI - dummy library

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as published
  by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
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
