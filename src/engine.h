/*
  LibRCC - Autodetection engines abstraction

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

#ifndef _RCC_ENGINE_H
#define _RCC_ENGINE_H

#include "../config.h"

#ifdef HAVE_RCD
# define RCC_RCD_SUPPORT
# undef RCC_RCD_DYNAMIC
#elif HAVE_DLOPEN
# define RCC_RCD_SUPPORT
# define RCC_RCD_DYNAMIC
#else
# undef RCC_RCD_SUPPORT
# undef RCC_RCD_DYNAMIC
#endif

#define RCC_RCD_LIB "librcd.so.0"

#ifdef RCC_RCD_DYNAMIC
# define RCC_RCD_SUPPORT
#endif

struct rcc_engine_context_t {
    rcc_language_config config;
    
    rcc_engine_function func;
    rcc_engine_free_function free_func;

    rcc_engine_id id;
    rcc_engine_internal internal;
};
typedef struct rcc_engine_context_t rcc_engine_context_s;

int rccEngineInit();
void rccEngineFree();

int rccEngineInitContext(rcc_engine_context engine_ctx, rcc_language_config config);
void rccEngineFreeContext(rcc_engine_context engine_ctx);
int rccEngineConfigure(rcc_engine_context ctx);

rcc_charset_id rccAutoengineRussian(rcc_engine_context ctx, const char *buf, int len);
rcc_autocharset_id rccEngineDetectCharset(rcc_engine_context ctx, const char *buf, size_t len);

#endif /* _RCC_ENGINE_H */
