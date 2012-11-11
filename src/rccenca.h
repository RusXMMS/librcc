/*
  LibRCC - interface to enca library

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

#ifndef _RCC_ENCA_H
#define _RCC_ENCA_H

#include "../config.h"

#ifdef HAVE_ENCA
# define RCC_ENCA_SUPPORT
# undef RCC_ENCA_DYNAMIC
#elif HAVE_DLOPEN
# define RCC_ENCA_SUPPORT
# define RCC_ENCA_DYNAMIC
#else
# undef RCC_ENCA_SUPPORT
# undef RCC_ENCA_DYNAMIC
#endif

#define RCC_ENCA_LIB "libenca.so.0"

#ifdef RCC_ENCA_DYNAMIC
# define RCC_ENCA_SUPPORT
#endif

struct rcc_enca_corrections_t {
    char *lang;
    const char *enca_charset;
    const char *iconv_charset;
};
typedef struct rcc_enca_corrections_t rcc_enca_corrections;


int rccEncaInit();
void rccEncaFree();

#endif /* _RCC_ENCA_H */
