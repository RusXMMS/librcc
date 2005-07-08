#include <stdio.h>
#include "../src/rccconfig.h"
#include "internal.h"
#include "rccnames.h"

#define RCC_UI_LOCK_CODE 0x1111

static rcc_ui_menu_context rccUiMenuCreateContext(rcc_ui_menu_type type, rcc_ui_id id, rcc_ui_context uictx) {
    rcc_ui_menu_context ctx;
    if ((!uictx)||(type>RCC_UI_MENU_MAX)) return NULL;
    
    ctx = (rcc_ui_menu_context)malloc(sizeof(rcc_ui_menu_context_s));
    if (!ctx) return ctx;
    
    ctx->uictx = uictx;
    ctx->type = type;
    ctx->id = id;
    
    ctx->widget = rccUiMenuCreateWidget(ctx);
    ctx->box = NULL;
    
    return ctx;
}

static void rccUiMenuFreeContext(rcc_ui_menu_context ctx) {
    if (!ctx) return;
    rccUiMenuFreeWidget(ctx);
    free(ctx);
}

static rcc_ui_frame_context rccUiFrameCreateContext(rcc_ui_frame_type type, rcc_ui_context uictx) {
    rcc_ui_frame_context ctx;
    if ((!uictx)||(type>RCC_UI_FRAME_MAX)) return NULL;
    
    ctx = (rcc_ui_frame_context)malloc(sizeof(rcc_ui_frame_context_s));
    if (!ctx) return ctx;
    
    ctx->uictx = uictx;
    ctx->type = type;
    
    ctx->frame = NULL;
    
    return ctx;
}

static void rccUiFrameFreeContext(rcc_ui_frame_context ctx) {
    if (!ctx) return;
    rccUiFrameFree(ctx);
    free(ctx);
}

rcc_ui_context rccUiCreateContext(rcc_context rccctx) {
    int err = 0;
    unsigned int i;
    
    rcc_class_ptr *classes;
    rcc_ui_context ctx;
    rcc_ui_menu_context *charsets;
    rcc_ui_menu_context *options;
    
    if (!rccctx) return NULL;

    err = rccLockConfiguration(rccctx, RCC_UI_LOCK_CODE);
    if (err) return NULL;
    
    classes = rccGetClassList(rccctx);
    for (i=0; classes[i]; i++);

    ctx = (rcc_ui_context)malloc(sizeof(struct rcc_ui_context_t));
    charsets = (rcc_ui_menu_context*)malloc((i+1)*sizeof(rcc_ui_menu_context));
    options = (rcc_ui_menu_context*)malloc((RCC_MAX_OPTIONS)*sizeof(rcc_ui_menu_context));
    if ((!ctx)||(!charsets)) {
	if (ctx) free(ctx);
	if (charsets) free(charsets);
	rccUnlockConfiguration(rccctx, RCC_UI_LOCK_CODE);
	return NULL;
    }

    ctx->options = options;
    ctx->charsets = charsets;
    ctx->rccctx = rccctx;
    
    ctx->language_names = NULL;
    ctx->option_names = NULL;

    ctx->internal = rccUiCreateInternal(ctx);

    ctx->language = rccUiMenuCreateContext(RCC_UI_MENU_LANGUAGE, 0, ctx);
    ctx->engine = rccUiMenuCreateContext(RCC_UI_MENU_ENGINE, 0, ctx);
    for (i=0; classes[i]; i++) {
        charsets[i] = rccUiMenuCreateContext(RCC_UI_MENU_CHARSET, i, ctx);
	if (!charsets[i]) err = 1;
    }
    charsets[i] = NULL;
    for (i=0; i<RCC_MAX_OPTIONS; i++) {
        options[i] = rccUiMenuCreateContext(RCC_UI_MENU_OPTION, i, ctx);
	if (!options[i]) err = 1;
    }

    ctx->language_frame = rccUiFrameCreateContext(RCC_UI_FRAME_LANGUAGE, ctx);
    ctx->charset_frame = rccUiFrameCreateContext(RCC_UI_FRAME_CHARSETS, ctx);
    ctx->engine_frame = rccUiFrameCreateContext(RCC_UI_FRAME_ENGINE, ctx);
    ctx->page = NULL;


    if ((err)||(!ctx->language)||(!ctx->engine)||(!ctx->language_frame)||(!ctx->charset_frame)||(!ctx->engine_frame)) {
	rccUiFreeContext(ctx);
	return NULL;
    }

    return ctx;
}

void rccUiFreeContext(rcc_ui_context ctx) {
    unsigned int i;
    rcc_class_ptr *classes;

    if (!ctx) return;
    
    rccUiFreeInternal(ctx);

    if (ctx->engine_frame) rccUiFrameFreeContext(ctx->engine_frame);
    if (ctx->charset_frame) rccUiFrameFreeContext(ctx->charset_frame);
    if (ctx->language_frame) rccUiFrameFreeContext(ctx->language_frame);
        
    if (ctx->charsets) {
	classes = rccGetClassList(ctx->rccctx);
	for (i=0; classes[i]; i++)
	    if (ctx->charsets[i]) rccUiMenuFreeContext(ctx->charsets[i]);
	free(ctx->charsets);
    }
    if (ctx->options) {
	for (i=0;i<RCC_MAX_OPTIONS;i++) {
	    if (ctx->options[i]) rccUiMenuFreeContext(ctx->options[i]);
	}
	free(ctx->options);
    }
    if (ctx->engine) rccUiMenuFreeContext(ctx->engine);
    if (ctx->language) rccUiMenuFreeContext(ctx->language);

    rccUnlockConfiguration(ctx->rccctx, RCC_UI_LOCK_CODE);

    free(ctx);
}

int rccUiSetLanguageNames(rcc_ui_context ctx, rcc_language_name *names) {
    if (!ctx) return -1;
    
    if (names) ctx->language_names = names;
    else ctx->language_names = rcc_default_language_names;
    return 0;
}

int rccUiSetOptionNames(rcc_ui_context ctx, rcc_option_name *names) {
    if (!ctx) return -1;

    if (names) ctx->option_names = names;
    else ctx->option_names = rcc_default_option_names;
    return 0;
}


int rccUiRestoreLanguage(rcc_ui_context ctx) {
    unsigned int i;
    rcc_class_ptr *classes;
    rcc_language_id language_id;
    
    if (!ctx) return -1;

    language_id = (rcc_language_id)rccUiMenuGet(ctx->language);
    
    rccUiMenuConfigureWidget(ctx->engine);
    //rccUiMenuSet(ctx->engine, (rcc_ui_id)rccConfigGetSelectedEngine(config));

    classes = rccGetClassList(ctx->rccctx);
    for (i=0;classes[i];i++) 
	if (classes[i]->fullname) {
	    rccUiMenuConfigureWidget(ctx->charsets[i]);
//	    rccUiMenuSet(ctx->charsets[i], rccConfigGetSelectedCharset(config, (rcc_class_id)i));
	}


    return 0;
}

int rccUiRestore(rcc_ui_context ctx) {
    unsigned int i;
    rcc_context rccctx;
    rcc_language_id language_id;
    
    if (!ctx) return -1;

    rccctx = ctx->rccctx;
    
    language_id = rccGetSelectedLanguage(rccctx);
    rccUiMenuSet(ctx->language, (rcc_ui_id)language_id);
    
    for (i=0;i<RCC_MAX_OPTIONS;i++)
	rccUiMenuSet(ctx->options[i], rccGetOption(rccctx, (rcc_option)i));

    return 0;
}

int rccUiUpdate(rcc_ui_context ctx) {
    unsigned int i;
    rcc_class_ptr *classes;
    rcc_context rccctx;
    
    if (!ctx) return -1;
    
    rccctx = ctx->rccctx;

    rccSetLanguage(rccctx, (rcc_language_id)rccUiMenuGet(ctx->language));

    for (i=0;i<RCC_MAX_OPTIONS;i++)
	rccSetOption(rccctx, (rcc_option)i, (rcc_option_value)rccUiMenuGet(ctx->options[i]));

    rccSetEngine(rccctx, (rcc_language_id)rccUiMenuGet(ctx->engine));

    classes = rccGetClassList(rccctx);
    for (i=0;classes[i];i++)
	if (classes[i]->fullname)
	    rccSetCharset(rccctx, (rcc_class_id)i, rccUiMenuGet(ctx->charsets[i])); 
    
    return 0;
}



rcc_ui_widget rccUiGetLanguageMenu(rcc_ui_context ctx) {
    if (!ctx) return NULL;
    
    if (rccUiMenuConfigureWidget(ctx->language)) return NULL;
    return ctx->language->widget;
}

rcc_ui_widget rccUiGetCharsetMenu(rcc_ui_context ctx, rcc_class_id id) {
    rcc_class_ptr *classes;
    unsigned int i;
    
    if ((!ctx)||(id<0)) return NULL;

    classes = rccGetClassList(ctx->rccctx);
    for (i=0;classes[i];i++);
    if (id>=i) return NULL;
    
    if (rccUiMenuConfigureWidget(ctx->charsets[id])) return NULL;
    return ctx->charsets[id]->widget;
}


rcc_ui_widget rccUiGetEngineMenu(rcc_ui_context ctx) {
    if (!ctx) return NULL;

    if (rccUiMenuConfigureWidget(ctx->engine)) return NULL;
    return ctx->engine->widget;
}


rcc_ui_widget rccUiGetOptionMenu(rcc_ui_context ctx, rcc_option option) {
    if ((!ctx)||(option<0)||(option>RCC_MAX_OPTIONS)) return NULL;

    if (rccUiMenuConfigureWidget(ctx->options[option])) return NULL;
    return ctx->options[option]->widget;
}


rcc_ui_box rccUiGetLanguageBox(rcc_ui_context ctx, const char *title) {
    rcc_ui_widget language;

    if (!ctx) return NULL;
    if (ctx->language->box) return ctx->language->box;
    
    language = rccUiGetLanguageMenu(ctx);
    if (!language) return NULL;
    
    ctx->language->box = rccUiBoxCreate(ctx->language, title);
    return ctx->language->box;
}

rcc_ui_box rccUiGetCharsetBox(rcc_ui_context ctx, rcc_class_id id, const char *title) {
    unsigned int i;
    rcc_class_ptr *classes;
    rcc_ui_widget charset;
    
    if (!ctx) return NULL;

    classes = rccGetClassList(ctx->rccctx);
    for (i=0; classes[i]; i++);
    if (id>=i) return NULL;

    if (ctx->charsets[id]->box) return ctx->charsets[id]->box;


    charset = rccUiGetCharsetMenu(ctx, id);
    if (!charset) return NULL;

    ctx->charsets[id]->box = rccUiBoxCreate(ctx->charsets[id], title);
    return ctx->charsets[id]->box;
}

rcc_ui_box rccUiGetEngineBox(rcc_ui_context ctx, const char *title) {
    rcc_ui_widget engine;

    if (!ctx) return NULL;
    if (ctx->engine->box) return ctx->engine->box;

    engine = rccUiGetEngineMenu(ctx);
    if (!engine) return NULL;

    ctx->engine->box = rccUiBoxCreate(ctx->engine, title);
    return ctx->engine->box;
}

rcc_ui_box rccUiGetOptionBox(rcc_ui_context ctx, rcc_option option, const char *title) {
    rcc_ui_widget opt;

    if ((!ctx)||(option<0)||(option>=RCC_MAX_OPTIONS)) return NULL;
    if (ctx->options[option]->box) return ctx->options[option]->box;

    opt = rccUiGetOptionMenu(ctx, option);
    if (!opt) return NULL;
    
    ctx->options[option]->box = rccUiBoxCreate(ctx->options[option], title);
    return ctx->options[option]->box;

}

rcc_ui_frame rccUiGetLanguageFrame(rcc_ui_context ctx, const char *title) {
    rcc_ui_frame_context framectx;
    rcc_ui_frame frame;
    rcc_ui_box language;

    if (!ctx) return NULL;
    
    framectx = ctx->language_frame;
    if (framectx->frame) return framectx->frame;
    
    frame = rccUiFrameCreate(ctx->language_frame, title);
    if (frame) framectx->frame = frame;
    else return NULL;
    
    language = rccUiGetLanguageBox(ctx, title);
    if (!language) return NULL;

    rccUiFrameAdd(framectx, language);
    
    return frame;
}

rcc_ui_frame rccUiGetCharsetsFrame(rcc_ui_context ctx, const char *title) {
    unsigned int i;
    rcc_class_ptr *classes;
    rcc_ui_frame_context framectx;
    rcc_ui_frame frame;
    rcc_ui_box charset;
    
    if (!ctx) return NULL;

    framectx = ctx->charset_frame;
    if (framectx->frame) return framectx->frame;

    frame = rccUiFrameCreate(framectx, title);
    if (frame) framectx->frame = frame;
    else return NULL;

    classes = rccGetClassList(ctx->rccctx);
    for (i=0; classes[i]; i++) {
	if (classes[i]->fullname) {
	    charset = rccUiGetCharsetBox(ctx, (rcc_class_id)i,  classes[i]->fullname);
	    rccUiFrameAdd(framectx, charset);
	}
    }
    
    
    return frame;
}


rcc_ui_frame rccUiGetEngineFrame(rcc_ui_context ctx, const char *title) {
    unsigned int i;
    rcc_ui_frame_context framectx;
    rcc_ui_frame frame;
    rcc_ui_box engine;
    rcc_ui_box opt;
    const char *optname;

    if (!ctx) return NULL;

    framectx = ctx->engine_frame;
    if (framectx->frame) return framectx->frame;
    
    frame = rccUiFrameCreate(framectx, title);
    if (frame) framectx->frame = frame;
    else return NULL;
    
    engine = rccUiGetEngineBox(ctx, title);
    rccUiFrameAdd(framectx, engine);

    for (i=0; i<RCC_MAX_OPTIONS; i++) {
	optname = rccUiGetOptionName(ctx, i);
	if (!optname) continue;
	
	opt = rccUiGetOptionBox(ctx, (rcc_option)i, optname);
	rccUiFrameAdd(framectx, opt);
    }
    
    return frame;
}


rcc_ui_page rccUiGetPage(rcc_ui_context ctx, const char *title, const char *language_title, const char *charset_title, const char *engine_title) {
    rcc_ui_page page;
    rcc_ui_frame frame;
    
    if (!ctx) return NULL;

    if (ctx->page) return ctx->page;

    page = rccUiPageCreate(ctx, title);
    if (!page) return NULL;

    frame = rccUiGetLanguageFrame(ctx, language_title);
    rccUiPageAdd(page, frame);

    frame = rccUiGetCharsetsFrame(ctx, charset_title);
    rccUiPageAdd(page, frame);

    frame = rccUiGetEngineFrame(ctx, engine_title);
    rccUiPageAdd(page, frame);
    
    ctx->page = page;
    
    return page;
}
