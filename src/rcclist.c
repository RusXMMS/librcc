/*
  LibRCC - module providing access to currently registered languages, engines,
  charsets

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
#include "internal.h"

rcc_language_ptr *rccGetLanguageList(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return ctx->languages;
}

rcc_charset *rccGetCharsetList(rcc_context ctx, rcc_language_id language_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    if (language_id>=ctx->n_languages) return NULL;
    if (!language_id) language_id = rccGetCurrentLanguage(ctx);

    return ctx->languages[language_id]->charsets;
}

rcc_engine_ptr *rccGetEngineList(rcc_context ctx, rcc_language_id language_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    if (language_id>=ctx->n_languages) return NULL;
    if (!language_id) language_id = rccGetCurrentLanguage(ctx);

    return ctx->languages[language_id]->engines;
}

rcc_charset *rccGetCurrentCharsetList(rcc_context ctx) {
    rcc_language_id language_id;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    language_id = rccGetCurrentLanguage(ctx);
    if (language_id == (rcc_language_id)-1) return NULL;

    return rccGetCharsetList(ctx, language_id);
}

rcc_engine_ptr *rccGetCurrentEngineList(rcc_context ctx) {
    rcc_language_id language_id;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    language_id = rccGetCurrentLanguage(ctx);
    if (language_id == (rcc_language_id)-1) return NULL;

    return rccGetEngineList(ctx, language_id); 
}

rcc_charset *rccGetCurrentAutoCharsetList(rcc_context ctx) {
    rcc_language_id language_id;
    rcc_engine_id engine_id;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    language_id = rccGetCurrentLanguage(ctx);
    engine_id = rccGetCurrentEngine(ctx);
    if ((language_id == (rcc_language_id)-1)||(engine_id == (rcc_engine_id)-1)) return NULL;
    
    
    return ctx->languages[language_id]->engines[engine_id]->charsets;
}

rcc_class_ptr *rccGetClassList(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return ctx->classes;
}
