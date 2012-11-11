/*
  LibRCC - LibRCD engine

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

#include <librcd.h>
#include <librcc.h>

static rcc_autocharset_id AutoengineRussian(rcc_engine_context ctx, const char *buf, int len) {
    return (rcc_autocharset_id)rcdGetRussianCharset(buf,len);
}

static rcc_engine russian_engine = {
    "LibRCD", NULL, NULL, &AutoengineRussian, {"CP1251","KOI8-R","UTF-8","IBM866", "ISO8859-1", NULL}
};

static rcc_engine ukrainian_engine = {
    "LibRCD", NULL, NULL, &AutoengineRussian, {"CP1251","KOI8-U","UTF-8","IBM865", "ISO8859-1", NULL}
};

static rcc_engine belarussian_engine = {
    "LibRCD", NULL, NULL, &AutoengineRussian, {"CP1251","ISO-IR-111","UTF-8","IBM866", "ISO8859-1", NULL}
};

rcc_engine *rccGetInfo(const char *lang) {
    if (!lang) return NULL;
    
    if (!strcmp(lang, "ru")) return &russian_engine;
    if (!strcmp(lang, "uk")) return &ukrainian_engine;
    if (!strcmp(lang, "be")) return &belarussian_engine;
    
    return NULL;
}
