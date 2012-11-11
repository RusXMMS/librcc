/*
  LibRCC - Autodetection engines abstraction

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

#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */

#include "internal.h"
#include "plugin.h"
#include "rccconfig.h"
#include "rccenca.h"

#include "engine.h"

#ifdef RCC_RCD_SUPPORT
# ifdef RCC_RCD_DYNAMIC
#  include "fake_rcd.h"
# else
#  include <librcd.h>
# endif /* RCC_RCD_DYNAMIC */
#endif /* RCC_RCD_SUPPORT */

#ifdef RCC_RCD_DYNAMIC
static rcc_library_handle rcd_handle = NULL;
#endif /* RCC_RCD_DYNAMIC */

rcc_autocharset_id rccAutoengineRussian(rcc_engine_context ctx, const char *buf, int len) {
#ifdef RCC_RCD_SUPPORT
# ifdef RCC_RCD_DYNAMIC
    if (!rcdGetRussianCharset) return (rcc_charset_id)-1;
# endif /* RCC_RCD_DYNAMIC */
    return (rcc_charset_id)rcdGetRussianCharset(buf,len);
#else /* RCC_RCD_SUPPORT */
    return (rcc_charset_id)-1;
#endif /* RCC_RCD_SUPPORT */
}


#ifdef RCC_RCD_DYNAMIC
static int rccRCDLibraryLoad() {
    if (rcd_handle) return 0;
    
    rcd_handle = rccLibraryOpen(RCC_RCD_LIB);
    if (!rcd_handle) return -1;

    rcdGetRussianCharset = rccLibraryFind(rcd_handle,"rcdGetRussianCharset");
    if (!rcdGetRussianCharset) {
        rccLibraryClose(rcd_handle);
        rcd_handle = NULL;
# ifdef RCC_DEBUG
	perror( "rccRCD. Incomplete function set in library" );
# endif /* RCC_DEBUG */
	return -1;
    }

    return 0;
}
#endif /* RCC_RCD_DYNAMIC */

#ifdef RCC_RCD_DYNAMIC
static void rccRCDLibraryUnload() {
    if (rcd_handle) {
	rccLibraryClose(rcd_handle);
	rcd_handle = NULL;
    }
}
#endif /* RCC_RCD_DYNAMIC */

int rccEngineInit() {
#ifdef RCC_RCD_DYNAMIC
    int err;
    unsigned int i,j,flag;
    rcc_engine **engines;
    int *charsets;
    
    err = rccRCDLibraryLoad();
    if (err) {
	for (i=0;rcc_default_languages[i].sn;i++) {
	    engines = rcc_default_languages[i].engines;
	    for (flag=0,j=0;engines[j];j++) {
		if (flag) engines[j-1] = engines[j];
		else if (engines[j] == &rcc_russian_engine) flag=1;
	    }
	    if (flag) engines[j-1] = NULL;
	}
    }
#endif /* RCC_RCD_DYNAMIC  */

    return rccEncaInit();
}

void rccEngineFree() {
    rccEncaFree();
#ifdef RCC_RCD_DYNAMIC
    rccRCDLibraryUnload();
#endif /* RCC_RCD_DYNAMIC */
}

int rccEngineInitContext(rcc_engine_context engine_ctx, rcc_language_config config) {
    if ((!config)||(!engine_ctx)) return -1;
    
    engine_ctx->config = config;
    engine_ctx->free_func = NULL;
    engine_ctx->func = NULL;
    return 0;
}

void rccEngineFreeContext(rcc_engine_context engine_ctx) {
    if (!engine_ctx) return;

    if (engine_ctx->free_func) {
	engine_ctx->free_func(engine_ctx);
	engine_ctx->free_func = NULL;
    }

    engine_ctx->func = NULL;
    engine_ctx->internal = NULL;
}

int rccEngineConfigure(rcc_engine_context ctx) {
    rcc_engine_id engine_id;
    rcc_engine *engine;

    if ((!ctx)||(!ctx->config)) return -1;

    rccEngineFreeContext(ctx);
    engine_id = rccConfigGetCurrentEngine(ctx->config);
    if (engine_id == (rcc_engine_id)-1) return -1;

    engine = ctx->config->language->engines[engine_id];
    
    ctx->id = engine_id;
    ctx->free_func = engine->free_func;
    ctx->func = engine->func;

    if (engine->init_func) ctx->internal = engine->init_func(ctx);
    else ctx->internal = NULL;
    
    return 0;
}


rcc_engine *rccEngineGetInfo(rcc_engine_context ctx) {
    if (!ctx) return NULL;
    return ctx->config->language->engines[ctx->id];
}

rcc_autocharset_id rccEngineGetAutoCharsetByName(rcc_engine_context ctx, const char *name) {
    unsigned int i;
    rcc_engine *info;
    rcc_charset *charsets;

    if ((!ctx)||(!name)) return (rcc_autocharset_id)-1;
    
    info = rccEngineGetInfo(ctx);
    if (info) {
	charsets = info->charsets;
    
	for (i=0;charsets[i];i++)
	    if (!strcasecmp(charsets[i],name)) return (rcc_autocharset_id)i;
    }
    
    return (rcc_autocharset_id)-1;
}

rcc_engine_internal rccEngineGetInternal(rcc_engine_context ctx) {
    if (!ctx) return NULL;

    return ctx->internal;
}

rcc_language *rccEngineGetLanguage(rcc_engine_context ctx) {
    if (!ctx) return NULL;

    return ctx->config->language;
}

rcc_context rccEngineGetRccContext(rcc_engine_context ctx) {
    if (!ctx) return NULL;

    return ctx->config->ctx;
}

#define bit(i) (1<<i)

static int CheckWestern(const unsigned char *buf, int len) {
    long i,j;
    int bytes=0;

    if (!len) len = strlen((char*)buf);
    for (i=0;i<len;i++) {
	if (bytes>0) {
		    // Western is 0x100-0x17e
	    if ((buf[i]&0xC0)==0x80) bytes--;
	    else return 0;
	} else {
	    if (buf[i]<128) continue;
	    
	    for (j=6;j>=0;j--)
		if ((buf[i]&bit(j))==0) break;

	    if ((j==0)||(j==6)) return 0;

	    bytes=6-j;
	    if (bytes==1) {
		// Western Languages (C2-C3)
		if ((buf[i]!=0xC2)&&(buf[i]!=0xC3)) return 0;
	    } else return 0;
	}
    }
    return 1;
}


rcc_autocharset_id rccEngineDetectCharset(rcc_engine_context ctx, const char *buf, size_t len) {
    rcc_autocharset_id utf;

	/* DS: This should be done directly in autoengines, otherwise we will
	fail to detect 7bit encodings */
    if (CheckWestern((const unsigned char*)buf, len)) {
	utf=rccConfigGetAutoCharsetByName(ctx->config, "UTF-8");
	if (utf != (rcc_autocharset_id)-1) return utf;
	utf=rccConfigGetAutoCharsetByName(ctx->config, "UTF8");
	if (utf != (rcc_autocharset_id)-1) return utf;
	utf=rccConfigGetAutoCharsetByName(ctx->config, "UTF_8");
	return utf;
    }
    
    if ((ctx)&&(ctx->func)) return ctx->func(ctx, buf, len);
    return (rcc_autocharset_id)-1;
}

