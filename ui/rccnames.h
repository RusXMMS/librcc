/*
  LibRCC - module providing various titles in appropriate language and encoding

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

#ifndef _RCC_UI_NAMES_C
#define _RCC_UI_NAMES_H

extern rcc_name rcc_default_language_names[];
extern rcc_name rcc_default_language_names_embeded[];
extern rcc_option_name rcc_default_option_names[];
extern rcc_option_name rcc_default_option_names_embeded[];
extern rcc_ui_page_name rcc_ui_default_page_name;
extern rcc_ui_page_name rcc_ui_default_page_name_embeded;
extern rcc_name *rcc_default_class_names;
extern rcc_name *rcc_default_charset_names;
extern rcc_name *rcc_default_engine_names;

rcc_name *rccUiGetLanguageRccName(rcc_ui_context ctx, const char *lang);
rcc_name *rccUiGetCharsetRccName(rcc_ui_context ctx, const char *charset);
rcc_name *rccUiGetEngineRccName(rcc_ui_context ctx, const char *engine);
rcc_name *rccUiGetClassRccName(rcc_ui_context ctx, const char *cl);

rcc_option_name *rccUiGetOptionRccName(rcc_ui_context ctx, rcc_option option);

#endif /* _RCC_UI_NAMES_H */
