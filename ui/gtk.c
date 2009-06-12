/*
  LibRCC UI - GTK library

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
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include <stdio.h>
#include <gtk/gtk.h>

#include <librcc.h>

#include "internal.h"
#include "rccnames.h"

#define TITLE_WIDTH 224
#define TITLE_HEIGHT 10
#define BOX_SPACING 1
#define BOX_BORDER 0
#define FRAME_SPACING 1
#define FRAME_BORDER 0
#define PAGE_SPACING 1

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
    GtkWidget *menu;
    
    if (!ctx) return (rcc_ui_id)-1;

    if (ctx->type == RCC_UI_MENU_OPTION) {
	switch (rccUiMenuGetRangeType(ctx)) {
	    
	    case RCC_OPTION_RANGE_TYPE_BOOLEAN:
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx->widget));
	    case RCC_OPTION_RANGE_TYPE_MENU:
		break;
	    case RCC_OPTION_RANGE_TYPE_RANGE:
	        return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ctx->widget));
	    default:
		return (rcc_ui_id)-1;
	}
    }
    
    menu = gtk_option_menu_get_menu(ctx->widget);
    return g_list_index(GTK_MENU_SHELL(menu)->children, gtk_menu_get_active(GTK_MENU(menu)));
}

int rccUiMenuSet(rcc_ui_menu_context ctx, rcc_ui_id id) {
    if (!ctx) return -1;
    
    switch (ctx->type) {
	case RCC_UI_MENU_OPTION:
	    switch (rccUiMenuGetRangeType(ctx)) {
		case RCC_OPTION_RANGE_TYPE_BOOLEAN:
		    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->widget),id);
		break;
		case RCC_OPTION_RANGE_TYPE_MENU:
		    gtk_option_menu_set_history(ctx->widget, id);
		break;
		case RCC_OPTION_RANGE_TYPE_RANGE:
		    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->widget), id);
		default:
		    return -1;
	    }	
	break;
	default:
	    gtk_option_menu_set_history(ctx->widget, id);
    }

    return 0;
}


static void rccGtkMenuLanguageCB(GtkWidget * w, gpointer item) {
    rccUiRestoreLanguage(((rcc_ui_menu_context)item)->uictx);
}

int rccUiMenuConfigureWidget(rcc_ui_menu_context ctx) {
    unsigned int i;
    unsigned long num;
    
    rcc_context rccctx;
    rcc_ui_context uictx;

    rcc_language_config config;
    rcc_language_id language_id;

    rcc_class_id class_id;
    rcc_charset_id charset_id;
    rcc_engine_id engine_id;

    const char *language;    
    const char *charset;
    const char *engine;
    
    rcc_option_range *range;
    rcc_option_name *option_name;
    rcc_option_value_names optnames;
    
    GtkWidget *list, *item, *menu;
    GtkObject *adjustment;

    if (!ctx) return -1;
    
    uictx = ctx->uictx;
    rccctx = uictx->rccctx;
    
    switch (ctx->type) {
	case RCC_UI_MENU_LANGUAGE:
	    num = rccGetLanguageNumber(rccctx);
	    
	    list = gtk_menu_new();
	    for (i=0; i<(num?num:1); i++) {
		language = rccUiGetLanguageName(uictx, (rcc_language_id)i);
		if (!language) continue;
		
		item = gtk_menu_item_new_with_label(language);
		gtk_widget_show(item);
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(rccGtkMenuLanguageCB), ctx);
		gtk_menu_append(GTK_MENU(list), item);
	    }

	    language_id = rccGetSelectedLanguage(rccctx);
	    if (language_id == (rcc_language_id)-1) language_id = 0;

	    if (ctx->widget) menu = ctx->widget;
	    else {
		menu = gtk_option_menu_new();
		ctx->widget = menu;
	    }
	    gtk_option_menu_remove_menu(GTK_OPTION_MENU(menu));
	    gtk_option_menu_set_menu(GTK_OPTION_MENU(menu), list);
	    gtk_option_menu_set_history(GTK_OPTION_MENU(menu), language_id);
	break;
	case RCC_UI_MENU_CHARSET:
	    language_id = (rcc_language_id)rccUiMenuGet(uictx->language);
	    class_id = rccUiMenuGetClassId(ctx);
	    config = rccGetConfig(rccctx, language_id);
	    num = rccConfigGetClassCharsetNumber(config, class_id);
	    
	    list = gtk_menu_new();
	    for (i=0;i<(num?num:1);i++) {
		charset = rccUiGetCharsetName(uictx, language_id, class_id, (rcc_charset_id)i);
		if (!charset) continue;
		
		item = gtk_menu_item_new_with_label(charset);
		if (rccIsDisabledCharsetName(rccctx, class_id, charset))
		    gtk_widget_set_sensitive(item, 0);
		else 
		    gtk_widget_show(item);
    		gtk_menu_append(GTK_MENU(list), item);
	    }

	    if (ctx->widget) menu = ctx->widget;
	    else {
		menu = gtk_option_menu_new();
		ctx->widget = menu;
	    }
    
	    gtk_option_menu_remove_menu(GTK_OPTION_MENU(menu));
	    gtk_option_menu_set_menu(GTK_OPTION_MENU(menu), list);

	    charset_id = rccConfigGetSelectedCharset(config, class_id);
	    if (charset_id == (rcc_charset_id)-1) charset_id = 0;
	    gtk_option_menu_set_history(GTK_OPTION_MENU(menu), charset_id);
	break;
	case RCC_UI_MENU_ENGINE:

	    language_id = (rcc_language_id)rccUiMenuGet(uictx->language);
	    config = rccGetConfig(rccctx, language_id);
	    num = rccConfigGetEngineNumber(config);

	    list = gtk_menu_new();
	    for (i=0;i<(num?num:1);i++) {
		engine = rccUiGetEngineName(uictx, language_id, (rcc_engine_id)i);
		if (!engine) continue;
		
		item = gtk_menu_item_new_with_label(engine);
		gtk_widget_show(item);
    		gtk_menu_append(GTK_MENU(list), item);
	    }	

	    if (ctx->widget) menu = ctx->widget;
	    else {
		menu = gtk_option_menu_new();
		ctx->widget = menu;
	    }

	    gtk_option_menu_remove_menu(GTK_OPTION_MENU(menu));
	    gtk_option_menu_set_menu(GTK_OPTION_MENU(menu), list);
	    engine_id = rccConfigGetCurrentEngine(config);
	    if (engine_id == (rcc_engine_id)-1) engine_id = 0;
	    gtk_option_menu_set_history(GTK_OPTION_MENU(menu), engine_id);
	break;
	case RCC_UI_MENU_OPTION:
	    switch (rccUiMenuGetRangeType(ctx)) {
		case RCC_OPTION_RANGE_TYPE_BOOLEAN:
		    if (!ctx->widget) {
		        item = gtk_check_button_new_with_label(rccUiGetOptionName(uictx, rccUiMenuGetOption(ctx)));
			ctx->widget = item;
		    }
    		    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->widget), rccGetOption(rccctx, rccUiMenuGetOption(ctx)));
		break;
		case RCC_OPTION_RANGE_TYPE_MENU:
		    if (!ctx->widget) {
			option_name = rccUiGetOptionRccName(uictx, rccUiMenuGetOption(ctx));
			if (!option_name) return -1;
			optnames = option_name->value_names;
			if (!optnames) return -1;

			list = gtk_menu_new();
			for (i=0;optnames[i];i++) {
			    item = gtk_menu_item_new_with_label(optnames[i]);
			    gtk_widget_show(item);
    			    gtk_menu_append(GTK_MENU(list), item);
			}
			
			menu = gtk_option_menu_new();
			ctx->widget = menu;

			gtk_option_menu_remove_menu(GTK_OPTION_MENU(menu));
			gtk_option_menu_set_menu(GTK_OPTION_MENU(menu), list);
		    }
		    gtk_option_menu_set_history(GTK_OPTION_MENU(ctx->widget), rccGetOption(rccctx, rccUiMenuGetOption(ctx)));
		break;
		case RCC_OPTION_RANGE_TYPE_RANGE:
		    range = rccUiMenuGetRange(ctx);
		    adjustment = gtk_adjustment_new(rccGetOption(rccctx, rccUiMenuGetOption(ctx)), range->min, range->max, range->step, range->step*5, range->step*5);
/*		    item = gtk_hscale_new(GTK_ADJUSTMENT(adjustment));
		    gtk_scale_set_digits(GTK_SCALE(item), 0);*/
		    item = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), range->step, 0);
		    ctx->widget = item;
		break;
		default:
		    return -1;
	    }
	break;
	default:
	    return -1;
    }

    return 0;
}


rcc_ui_box rccUiBoxCreate(rcc_ui_menu_context ctx, const char *title) {
    GtkWidget *hbox, *label;
    hbox = gtk_hbox_new(FALSE, BOX_SPACING);
    gtk_container_border_width(GTK_CONTAINER(hbox), BOX_BORDER);
    if ((ctx->type != RCC_UI_MENU_OPTION)||(rccUiMenuGetRangeType(ctx) != RCC_OPTION_RANGE_TYPE_BOOLEAN)) {
	label = gtk_label_new(title);
	gtk_widget_show(label);
	gtk_widget_set_usize(label, TITLE_WIDTH, TITLE_HEIGHT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }
    gtk_widget_show((GtkWidget*)ctx->widget);
    gtk_box_pack_start(GTK_BOX(hbox), (GtkWidget*)ctx->widget, TRUE, TRUE, 0);
    return (rcc_ui_box)hbox;
}

rcc_ui_frame rccUiFrameCreate(rcc_ui_frame_context ctx, const char *title) {
    GtkWidget *frame, *box;

    if (!ctx) return NULL;
    
    frame = gtk_frame_new(title?title:"");
    gtk_container_border_width(GTK_CONTAINER(frame), FRAME_BORDER);

    box = gtk_vbox_new(FALSE, FRAME_SPACING);
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(frame), box);

    return (rcc_ui_frame)frame;
}

void rccUiFrameFree(rcc_ui_frame_context ctx) {
}


int rccUiFrameAdd(rcc_ui_frame_context ctx, rcc_ui_box box) {
    GtkWidget *vbox;
    
    if ((!ctx)||(!box)) return -1;
    
    vbox = gtk_container_children(GTK_CONTAINER(ctx->frame))->data;
    gtk_widget_show(GTK_WIDGET(box));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(box), FALSE, FALSE, 0);
    return 0;
}

rcc_ui_page rccUiPageCreate(rcc_ui_context ctx, const char *title) {
    GtkWidget *vbox;
    vbox = gtk_vbox_new(FALSE, PAGE_SPACING);
    return (rcc_ui_page)vbox;
}

int rccUiPageAdd(rcc_ui_page page, rcc_ui_frame frame) {
    if ((!page)||(!frame)) return -1;
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(page), GTK_WIDGET(frame), FALSE, FALSE, 0);
    return 0;
}
