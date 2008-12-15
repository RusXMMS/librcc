/*
  LibRCC - module providing file names recoding

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

#ifndef _RCC_FS_H
#define _RCC_FS_H

int rccFS0(rcc_language_config config, const char *fspath, const char *filename, char **prefix, char **name);
int rccFS1(rcc_language_config config, const char *fspath, char **prefix, char **name);
char *rccFS2(rcc_language_config config, iconv_t icnv, const char *prefix, const char *name);
char *rccFS3(rcc_language_config config, rcc_class_id class_id, const char *prefix, const char *name);
char *rccFS5(rcc_context ctx, rcc_language_config config, rcc_class_id class_id, const char *utfstring);

#endif /* _RCC_FS_H */
