/*
  LibRCC - language configuration: supported charsets and engines, various
  options

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

#ifndef _RCC_LNGCONFIG_H
#define _RCC_LNGCONFIG_H

#include "rcciconv.h"
#include "rcctranslate.h"
#include "rccspell.h"
#include "rccmutex.h"

struct rcc_language_config_t {
    rcc_context ctx;
    rcc_language *language;
    
    rcc_engine_id engine;
    rcc_charset_id *charset;
    rcc_charset_id *default_charset;

    rcc_iconv *iconv_to;
    unsigned char configure;
    
    unsigned char configured;

    rcc_speller speller;
    rcc_translate trans;
    rcc_language_id translang;
    rcc_translate entrans;
    
    rcc_iconv fsiconv;

    rcc_engine_context_s engine_ctx;
    rcc_mutex mutex;
};
typedef struct rcc_language_config_t rcc_language_config_s;

rcc_language_ptr rccConfigGetLanguagePointer(rcc_language_config config);

rcc_engine_ptr rccConfigGetEnginePointer(rcc_language_config config, rcc_engine_id engine_id);
rcc_engine_ptr rccConfigCheckEnginePointer(rcc_language_config config, rcc_engine_id engine_id);
rcc_engine_ptr rccConfigGetCurrentEnginePointer(rcc_language_config config);
rcc_engine_ptr rccConfigCheckCurrentEnginePointer(rcc_language_config config);

rcc_speller rccConfigGetSpeller(rcc_language_config config);
rcc_translate rccConfigGetTranslator(rcc_language_config config, rcc_language_id to);
rcc_translate rccConfigGetEnglishTranslator(rcc_language_config config);

int rccConfigInit(rcc_language_config config, rcc_context ctx);
void rccConfigClear(rcc_language_config config);

int rccConfigConfigure(rcc_language_config config);

const char *rccConfigGetAutoCharsetName(rcc_language_config config, rcc_autocharset_id charset_id);
rcc_autocharset_id rccConfigGetAutoCharsetByName(rcc_language_config config, const char *name);

const char *rccGetAutoCharsetName(rcc_context ctx, rcc_autocharset_id charset_id);
rcc_autocharset_id rccGetAutoCharsetByName(rcc_context ctx, const char *name);

#endif /* _RCC_LNGCONFIG_H */
