/*
  LibRCC - base module for language manipulations

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

#ifndef _RCC_LNG_H
#define _RCC_LNG_H

#include "internal.h"
#include "lngconfig.h"


int rccCheckLanguageUsability(rcc_context ctx, rcc_language_id language_id);
rcc_language_ptr rccGetLanguagePointer(rcc_context ctx, rcc_language_id language_id);

#define rccGetCurrentEnginePointer(ctx) rccConfigGetCurrentEnginePointer(ctx->current_config)
#define rccCheckCurrentEnginePointer(ctx) rccConfigCheckCurrentEnginePointer(ctx->current_config)
#define rccGetEnginePointer(ctx,id) rccConfigGetEnginePointer(ctx->current_config, id)
#define rccCheckEnginePointer(ctx,id) rccConfigCheckEnginePointer(ctx->current_config, id)

#endif /* _RCC_LNG_H */
