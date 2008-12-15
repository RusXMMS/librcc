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

#ifndef _RCC_LIST_H
#define _RCC_LIST_H

rcc_language_ptr *rccGetLanguageList(rcc_context ctx);
rcc_charset *rccGetCharsetList(rcc_context ctx, rcc_language_id language_id);
rcc_engine_ptr *rccGetEngineList(rcc_context ctx, rcc_language_id language_id);
rcc_charset *rccGetCurrentCharsetList(rcc_context ctx);
rcc_engine_ptr *rccGetCurrentEngineList(rcc_context ctx);
rcc_charset *rccGetCurrentAutoCharsetList(rcc_context ctx);
rcc_class_ptr *rccGetClassList(rcc_context ctx);

#endif /* _RCC_LIST_H */
