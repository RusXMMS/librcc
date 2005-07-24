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
