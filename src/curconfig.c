/*
  LibRCC - module providing access to actual RCC configuration

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

int rccGetCharsetNumber(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return 0;
    }
    
    return rccConfigGetCharsetNumber(ctx->current_config);
}

int rccGetClassCharsetNumber(rcc_context ctx, rcc_class_id class_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return 0;
    }

    return rccConfigGetClassCharsetNumber(ctx->current_config, class_id);
}

int rccGetEngineNumber(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return 0;
    }
    
    return rccConfigGetEngineNumber(ctx->current_config);
}


const char *rccGetEngineName(rcc_context ctx, rcc_engine_id engine_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return rccConfigGetEngineName(ctx->current_config, engine_id);
}

const char *rccGetCharsetName(rcc_context ctx, rcc_charset_id charset_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return rccConfigGetCharsetName(ctx->current_config, charset_id);
}

const char *rccGetClassCharsetName(rcc_context ctx, rcc_class_id class_id, rcc_charset_id charset_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return rccConfigGetClassCharsetName(ctx->current_config, class_id, charset_id);
}

const char *rccGetAutoCharsetName(rcc_context ctx, rcc_autocharset_id charset_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return rccConfigGetAutoCharsetName(ctx->current_config, charset_id);
}

rcc_engine_id rccGetEngineByName(rcc_context ctx, const char *name) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_engine_id)-1;
    }

    return rccConfigGetEngineByName(ctx->current_config, name);
}

rcc_charset_id rccGetCharsetByName(rcc_context ctx, const char *name) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_charset_id)-1;
    }

    return rccConfigGetCharsetByName(ctx->current_config, name);
}

rcc_charset_id rccGetClassCharsetByName(rcc_context ctx, rcc_class_id class_id, const char *name) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_charset_id)-1;
    }

    return rccConfigGetClassCharsetByName(ctx->current_config, class_id, name);
}

rcc_autocharset_id rccGetAutoCharsetByName(rcc_context ctx, const char *name) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_autocharset_id)-1;
    }

    return rccConfigGetAutoCharsetByName(ctx->current_config, name);
}

rcc_engine_id rccGetSelectedEngine(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_engine_id)-1;
    }

    return rccConfigGetSelectedEngine(ctx->current_config);
}

const char *rccGetSelectedEngineName(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return rccConfigGetSelectedEngineName(ctx->current_config);
}

rcc_engine_id rccGetCurrentEngine(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_engine_id)-1;
    }
    return rccConfigGetCurrentEngine(ctx->current_config);
}

const char *rccGetCurrentEngineName(rcc_context ctx) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    return rccConfigGetCurrentEngineName(ctx->current_config);
}

rcc_charset_id rccGetSelectedCharset(rcc_context ctx, rcc_class_id class_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_charset_id)-1;
    }
    return rccConfigGetSelectedCharset(ctx->current_config, class_id);
}

const char *rccGetSelectedCharsetName(rcc_context ctx, rcc_class_id class_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    return rccConfigGetSelectedCharsetName(ctx->current_config, class_id);
}

rcc_charset_id rccGetCurrentCharset(rcc_context ctx, rcc_class_id class_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_charset_id)-1;
    }
    return rccConfigGetCurrentCharset(ctx->current_config, class_id);
}

const char *rccGetCurrentCharsetName(rcc_context ctx, rcc_class_id class_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }
    return rccConfigGetCurrentCharsetName(ctx->current_config, class_id);
}

int rccSetEngine(rcc_context ctx, rcc_engine_id engine_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    return rccConfigSetEngine(ctx->current_config, engine_id);
}

int rccSetCharset(rcc_context ctx, rcc_class_id class_id, rcc_charset_id charset_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    return rccConfigSetCharset(ctx->current_config, class_id, charset_id);
}

int rccSetEngineByName(rcc_context ctx, const char *name) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    return rccConfigSetEngineByName(ctx->current_config, name);
}

int rccSetCharsetByName(rcc_context ctx, rcc_class_id class_id, const char *name) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    return rccConfigSetCharsetByName(ctx->current_config, class_id, name);
}

rcc_charset_id rccGetLocaleCharset(rcc_context ctx, const char *locale_variable) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    return rccConfigGetLocaleCharset(ctx->current_config, locale_variable);
}

rcc_charset_id rccGetLocaleClassCharset(rcc_context ctx, rcc_class_id class_id, const char *locale_variable) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    return rccConfigGetLocaleClassCharset(ctx->current_config, class_id, locale_variable);
}

rcc_autocharset_id rccDetectCharset(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    return rccConfigDetectCharset(ctx->current_config, class_id, buf, len);
}

int rccIsDisabledCharset(rcc_context ctx, rcc_class_id class_id, rcc_charset_id charset_id) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    return rccConfigIsDisabledCharset(ctx->current_config, class_id, charset_id);
}
