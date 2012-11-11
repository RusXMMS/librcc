/*
  LibRCC - module providing file names recoding

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

#ifndef _RCC_FS_H
#define _RCC_FS_H

int rccFS0(rcc_language_config config, const char *fspath, const char *filename, char **prefix, char **name);
int rccFS1(rcc_language_config config, const char *fspath, char **prefix, char **name);
char *rccFS2(rcc_language_config config, iconv_t icnv, const char *prefix, const char *name);
char *rccFS3(rcc_language_config config, rcc_class_id class_id, const char *prefix, const char *name);
char *rccFS5(rcc_context ctx, rcc_language_config config, rcc_class_id class_id, const char *utfstring);

#endif /* _RCC_FS_H */
