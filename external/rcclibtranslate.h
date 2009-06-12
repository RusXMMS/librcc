/*
  LibRCC - external module interfacying libtranslate library

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

#ifndef _RCC_LIBTRANSLATE_H
#define _RCC_LIBTRANSLATE_H

int rccLibTranslateInit(const char *rcc_home_dir);
void rccLibTranslateFree();

void *rccLibTranslate(void *info);

#endif /* _RCC_LIBTRANSLATE_H */
