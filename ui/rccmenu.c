/*
  LibRCC - menu abstraction

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
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "rccmenu.h"

rcc_ui_menu_context rccUiMenuCreateContext(rcc_ui_menu_type type, rcc_ui_context uictx) {
    rcc_ui_menu_context ctx;
    if ((!uictx)||(type>=RCC_UI_MENU_MAX)) return NULL;
    
    ctx = (rcc_ui_menu_context)malloc(sizeof(rcc_ui_menu_context_s));
    if (!ctx) return ctx;
    
    ctx->uictx = uictx;
    ctx->type = type;
    
    ctx->widget = rccUiMenuCreateWidget(ctx);
    ctx->box = NULL;
    
    return ctx;
}

rcc_ui_menu_context rccUiCharsetMenuCreateContext(rcc_ui_menu_type type, rcc_charset_id id, rcc_ui_context uictx) {
    rcc_ui_charset_menu_context ctx;
    
    if ((!uictx)||(type>=RCC_UI_MENU_MAX)) return NULL;
    
    ctx = (rcc_ui_charset_menu_context)malloc(sizeof(rcc_ui_charset_menu_context_s));
    if (!ctx) return NULL;
    
    ctx->ui_menu.uictx = uictx;
    ctx->ui_menu.type = type;
    ctx->id = id;
    
    ctx->ui_menu.widget = rccUiMenuCreateWidget((rcc_ui_menu_context)ctx);
    ctx->ui_menu.box = NULL;
    
    return (rcc_ui_menu_context)ctx;
}

rcc_ui_menu_context rccUiOptionMenuCreateContext(rcc_ui_menu_type type, rcc_option id, rcc_option_type otype, rcc_option_range *range, rcc_ui_context uictx) {
    rcc_ui_option_menu_context ctx;
    
    if ((!uictx)||(type>=RCC_UI_MENU_MAX)) return NULL;
    
    ctx = (rcc_ui_option_menu_context)malloc(sizeof(rcc_ui_option_menu_context_s));
    if (!ctx) return NULL;
    
    ctx->ui_menu.uictx = uictx;
    ctx->ui_menu.type = type;
    ctx->id = id;
    ctx->type = otype;
    ctx->realtype = otype;
    ctx->range = range;
    
    ctx->ui_menu.widget = rccUiMenuCreateWidget((rcc_ui_menu_context)ctx);
    ctx->ui_menu.box = NULL;
    
    return (rcc_ui_menu_context)ctx;
}


void rccUiMenuFreeContext(rcc_ui_menu_context ctx) {
    if (!ctx) return;
    rccUiMenuFreeWidget(ctx);
    free(ctx);
}


rcc_class_id rccUiMenuGetClassId(rcc_ui_menu_context ctx) {
    if ((!ctx)||(ctx->type != RCC_UI_MENU_CHARSET)) return (rcc_class_id)-1;
    return ((rcc_ui_charset_menu_context)ctx)->id;
}

rcc_option rccUiMenuGetOption(rcc_ui_menu_context ctx) {
    if ((!ctx)||(ctx->type != RCC_UI_MENU_OPTION)) return (rcc_option)-1;
    return ((rcc_ui_option_menu_context)ctx)->id;
}


rcc_option_type rccUiMenuGetType(rcc_ui_menu_context ctx) {
    if ((!ctx)||(ctx->type != RCC_UI_MENU_OPTION)) return (rcc_option_type)-1;
    return ((rcc_ui_option_menu_context)ctx)->type;
}

rcc_option_range *rccUiMenuGetRange(rcc_ui_menu_context ctx) {
    if  ((!ctx)||(ctx->type != RCC_UI_MENU_OPTION)) return NULL;
    return ((rcc_ui_option_menu_context)ctx)->range;
}

rcc_option_range_type rccUiMenuGetRangeType(rcc_ui_menu_context ctx) {
    if  ((!ctx)||(ctx->type != RCC_UI_MENU_OPTION)) return (rcc_option_type)-1;
    return ((rcc_ui_option_menu_context)ctx)->range->type;
}

int rccUiMenuHide(rcc_ui_menu_context ctx) {
    if (!ctx) return -1;
    
	// Only options right now
    if (ctx->type != RCC_UI_MENU_OPTION) return -1;

    ((rcc_ui_option_menu_context)ctx)->type = RCC_OPTION_TYPE_INVISIBLE;

    return 0;
}

int rccUiMenuUnHide(rcc_ui_menu_context ctx) {
    if (!ctx) return -1;

	// Only options right now
    if (ctx->type != RCC_UI_MENU_OPTION) return -1;

    if (((rcc_ui_option_menu_context)ctx)->type == RCC_OPTION_TYPE_INVISIBLE) {
	if (((rcc_ui_option_menu_context)ctx)->realtype == RCC_OPTION_TYPE_INVISIBLE)
	    ((rcc_ui_option_menu_context)ctx)->type = RCC_OPTION_TYPE_STANDARD;
	else
	    ((rcc_ui_option_menu_context)ctx)->type = ((rcc_ui_option_menu_context)ctx)->realtype;
    }
    
    return 0;
}
