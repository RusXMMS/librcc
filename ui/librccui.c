#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>

#include "../config.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <librcc.h>

#include "internal.h"
#include "rccnames.h"

#define RCC_UI_LOCK_CODE 0x1111

#define XPATH_LANGUAGE "//Languages/Language[@name]"
#define XPATH_OPTION "//Options/Option[@name]"
#define XPATH_VALUE "//Options/Option[@name=\"%s\"]/Value[@name]"
#define XPATH_LANGUAGE_REQUEST_LOCALE "//Languages/Language[@name=\"%s\"]/FullName[@locale=\"%s\"]"
#define XPATH_LANGUAGE_REQUEST "//Languages/Language[@name=\"%s\"]/FullName"
#define XPATH_OPTION_REQUEST_LOCALE "//Options/Option[@name=\"%s\"]/FullName[@locale=\"%s\"]"
#define XPATH_OPTION_REQUEST "//Options/Option[@name=\"%s\"]/FullName"
#define XPATH_VALUE_REQUEST_LOCALE "//Options/Option[@name=\"%s\"]/Value[@name=\"%s\"]/FullName[@locale=\"%s\"]"
#define XPATH_VALUE_REQUEST "//Options/Option[@name=\"%s\"]/Value[@name=\"%s\"]/FullName"

static const char *rccUiXmlGetText(xmlNodePtr node) {
    if ((node)&&(node->children)&&(node->children->type == XML_TEXT_NODE)&&(node->children->content)) return node->children->content;
}

static xmlNodePtr rccUiNodeFind(xmlXPathContextPtr xpathctx, const char *request, ...) {
    xmlXPathObjectPtr obj;
    xmlNodeSetPtr node_set;
    xmlNodePtr res = NULL;

    unsigned int i, args = 0;
    unsigned int size = 128;
    va_list ap;
    char *req;
    
    if (!xpathctx) return NULL;

    for (req = strstr(request, "%s"); req; req = strstr(req + 1, "%s")) args++;
    
    if (args) {
	va_start(ap, request);
	for (i=0;i<args;i++) {
	    req = va_arg(ap, char*);
	    size += strlen(req);
	}
	va_end(ap);
	
	req = (char*)malloc(size*sizeof(char));
	if (!req) return NULL;
	
	va_start(ap, request);
	vsprintf(req,request,ap);
	va_end(ap);
    } else req = (char*)request;
    
    obj = xmlXPathEvalExpression(req, xpathctx);
    if (obj) {
	node_set = obj->nodesetval;
	if ((node_set)&&(node_set->nodeNr > 0)) {
	    res = node_set->nodeTab[0];
	}
	xmlXPathFreeObject(obj);
    }

    if (args) free(req);

    return res;
}

int rccUiInit() {
    int err;
    unsigned long i, j, k, nnodes;

    xmlDocPtr xmlctx;
    xmlXPathContextPtr xpathctx = NULL;    
    xmlXPathObjectPtr obj;
    xmlNodeSetPtr node_set;

    xmlNodePtr node, cnode;
    xmlAttrPtr attr;

    rcc_language_name *lang_name;
    const char *lang, *fullname;
    const char *locale;
    char *lpos;
    char *search[4];

    rcc_option option;
    const char *opt, *val;
    rcc_option_name *option_name;
    const char *value_name;
    
    unsigned int npos;

    size_t newsize;    
    char tmpbuf[RCC_UI_MAX_STRING_CHARS+1];
    char ctype_charset[32];
    rcc_iconv icnv;

    err = rccInit();
    if (err) return err;
    
    if (rccLocaleGetCharset(ctype_charset, NULL, 32)) icnv = NULL;
    else {
	if ((!strcasecmp(ctype_charset, "UTF-8"))||(!strcasecmp(ctype_charset, "UTF8"))) icnv = NULL;
	else icnv = rccIConvOpen(ctype_charset, "UTF-8");
    }
    
    locale = setlocale(LC_CTYPE, NULL);
    if (locale) {
	search[0] = strdup(locale);
	if (!search[0]) goto clean;
	lpos = strrchr(search[0], '@');
	if (lpos) *lpos = 0;

	lpos = strrchr(search[0], '.');
	if (lpos) {
	    search[1] = strdup(search[0]);
	    if (!search[1]) goto clean;
	    *strchr(search[1], '.') = 0;
    
	    lpos = strrchr(search[1], '_');
	    if (lpos) {
		search[2] = strdup(search[1]);
		if (!search[2]) goto clean;
		*strchr(search[2],'_') = 0;
		search[3] = NULL;
	    } else search[2] = NULL;
	} else search[1] = NULL;
    } else search[0] = NULL;
	
    for (npos = 0; rcc_default_language_names[npos].sn; npos++);
    
    xmlctx = (xmlDocPtr)rccGetConfiguration();
    if (xmlctx) xpathctx = xmlXPathNewContext(xmlctx);
    else xpathctx = NULL;
    if (xpathctx) {
	obj = xmlXPathEvalExpression(XPATH_LANGUAGE, xpathctx);
	if (obj) {
	    node_set = obj->nodesetval;
	    if (node_set) nnodes = node_set->nodeNr;
	    else nnodes = 0;
	} else nnodes = 0;
	
        for (i=0;i<nnodes;i++) {
	    node = node_set->nodeTab[i];
	    attr = xmlHasProp(node, "name");
	    lang = attr->children->content;
	    
	    if ((!lang)||(!lang[0])) continue;
	    
	    for (j=0, node = NULL;((search[j])&&(!node));j++)
		node = rccUiNodeFind(xpathctx, XPATH_LANGUAGE_REQUEST_LOCALE, lang, search[j]);
	    if (!node) {
		node = rccUiNodeFind(xpathctx, XPATH_LANGUAGE_REQUEST, lang);
		if (!node) continue;
	    }
	    
	    fullname = rccUiXmlGetText(node);
	    if (!fullname) continue;

	    if (icnv) {
		newsize = rccIConvRecode(icnv, tmpbuf, RCC_UI_MAX_STRING_CHARS, fullname, 0);
		if (newsize != (size_t)-1) {
		    cnode = xmlNewChild(node->parent, NULL, "Recoded", tmpbuf);
		    fullname = rccUiXmlGetText(cnode);
		    if (!fullname) fullname = rccUiXmlGetText(node);
		}
	    }
	    
	    lang_name = rccUiGetLanguageRccName(NULL, lang);
	    if (lang_name) lang_name->name = fullname;
	    else if (npos<RCC_MAX_LANGUAGES) {
		rcc_default_language_names[npos].sn = lang;
		rcc_default_language_names[npos].name = fullname;
		rcc_default_language_names[++npos].sn = NULL;
		rcc_default_language_names[npos].name = NULL;
	    }
	}
	
	if (obj) xmlXPathFreeObject(obj);
	
	obj = xmlXPathEvalExpression(XPATH_OPTION, xpathctx);
	if (obj) {
	    node_set = obj->nodesetval;
	    if (node_set) nnodes = node_set->nodeNr;
	    else nnodes = 0;
	} else nnodes = 0;
	
        for (i=0;i<nnodes;i++) {
	    node = node_set->nodeTab[i];
	    attr = xmlHasProp(node, "name");
	    opt = attr->children->content;

	    if ((!opt)||(!opt[0])) continue;
	    option = rccGetOptionByName(opt);
	    if (option == (rcc_option)-1) continue;
	    option_name = rccUiGetOptionRccName(NULL, option);
	    if (!option_name) continue;
	    
	    for (j=0, node = NULL;((search[j])&&(!node));j++)
		node = rccUiNodeFind(xpathctx, XPATH_OPTION_REQUEST_LOCALE, opt, search[j]);
	    if (!node) {
		node = rccUiNodeFind(xpathctx, XPATH_OPTION_REQUEST, opt);
	    }

	    if (node) {
		fullname = rccUiXmlGetText(node);
		if (fullname) {
		    if (icnv) {
			newsize = rccIConvRecode(icnv, tmpbuf, RCC_UI_MAX_STRING_CHARS, fullname, 0);
			if (newsize != (size_t)-1) {
			    cnode = xmlNewChild(node->parent, NULL, "Recoded", tmpbuf);
			    fullname = rccUiXmlGetText(cnode);
			    if (!fullname) fullname = rccUiXmlGetText(node);
			}
		    }
		    option_name->name = fullname;
		}
	    }
	    
	    if (!option_name->value_names) continue;
	    
	    for (k=0;option_name->value_names[k];k++) {
		value_name = rccGetOptionValueName(option, (rcc_option_value)k);
		if (!value_name) continue;

		for (j=0, node = NULL;((search[j])&&(!node));j++)
		    node = rccUiNodeFind(xpathctx, XPATH_VALUE_REQUEST_LOCALE, opt, value_name, search[j]);
		if (!node) {
		    node = rccUiNodeFind(xpathctx, XPATH_VALUE_REQUEST, opt, value_name);
		}	
	    
		if (node) {
		    fullname = rccUiXmlGetText(node);
		    if (fullname) {
			if (icnv) {
			    newsize = rccIConvRecode(icnv, tmpbuf, RCC_UI_MAX_STRING_CHARS, fullname, 0);
			    if (newsize != (size_t)-1) {
				cnode = xmlNewChild(node->parent, NULL, "Recoded", tmpbuf);
				fullname = rccUiXmlGetText(cnode);
				if (!fullname) fullname = rccUiXmlGetText(node);
			    }
			}
			option_name->value_names[k] = fullname;
		    }
		}
	    }
	}
	if (obj) xmlXPathFreeObject(obj);
	
	xmlXPathFreeContext(xpathctx);
    }

clean:
    for (j=0;search[j];j++) free(search[j]);
    if (icnv) rccIConvClose(icnv);

    return 0;
}

void rccUiFree() {
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
    rcc_option_type otype;
    rcc_option_range *orange;
    
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

    ctx->language = rccUiMenuCreateContext(RCC_UI_MENU_LANGUAGE, ctx);
    ctx->engine = rccUiMenuCreateContext(RCC_UI_MENU_ENGINE, ctx);
    for (i=0; classes[i]; i++) {
        charsets[i] = rccUiCharsetMenuCreateContext(RCC_UI_MENU_CHARSET, (rcc_charset_id)i, ctx);
	if (!charsets[i]) err = 1;
    }
    charsets[i] = NULL;
    for (i=0; i<RCC_MAX_OPTIONS; i++) {
	otype = rccOptionGetType(rccctx, (rcc_option)i);
	orange = rccOptionGetRange(rccctx, (rcc_option)i);
        options[i] = rccUiOptionMenuCreateContext(RCC_UI_MENU_OPTION, (rcc_option)i, otype, orange, ctx);
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

    ctx->language_names = names;
    return 0;
}

int rccUiSetOptionNames(rcc_ui_context ctx, rcc_option_name *names) {
    if (!ctx) return -1;

    ctx->option_names = names;
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
    
    for (i=0;i<RCC_MAX_OPTIONS;i++) {
	if (rccUiMenuGetType(ctx->options[i]) == RCC_OPTION_TYPE_INVISIBLE) continue;

	rccUiMenuSet(ctx->options[i], rccGetOption(rccctx, (rcc_option)i));
    }

    return 0;
}

int rccUiUpdate(rcc_ui_context ctx) {
    unsigned int i;
    rcc_class_ptr *classes;
    rcc_context rccctx;
    
    if (!ctx) return -1;
    
    rccctx = ctx->rccctx;

    rccSetLanguage(rccctx, (rcc_language_id)rccUiMenuGet(ctx->language));

    for (i=0;i<RCC_MAX_OPTIONS;i++) {
	if (rccUiMenuGetType(ctx->options[i]) == RCC_OPTION_TYPE_INVISIBLE) continue;
	rccSetOption(rccctx, (rcc_option)i, (rcc_option_value)rccUiMenuGet(ctx->options[i]));
    }

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
    rcc_option_name *optname;

    if (!ctx) return NULL;

    framectx = ctx->engine_frame;
    if (framectx->frame) return framectx->frame;
    
    frame = rccUiFrameCreate(framectx, title);
    if (frame) framectx->frame = frame;
    else return NULL;
    
    engine = rccUiGetEngineBox(ctx, title);
    rccUiFrameAdd(framectx, engine);

    for (i=0; i<RCC_MAX_OPTIONS; i++) {
	if (rccUiMenuGetType(ctx->options[i]) == RCC_OPTION_TYPE_INVISIBLE) continue;

	optname = rccUiGetOptionRccName(ctx, i);
	if (!optname) continue;
	
	opt = rccUiGetOptionBox(ctx, (rcc_option)i, optname->name);
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
