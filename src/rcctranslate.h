/*
  LibRCC - module responsible for translation service

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

#ifndef _RCC_TRANSLATE_H
#define _RCC_TRANSLATE_H

#include "rccmutex.h"
#include "rccexternal.h"
#define RCC_EXTERNAL_COMMAND_TRANSLATE 0x80
#define RCC_EXTERNAL_COMMAND_TRANSLATE_QUEUE 0x81


struct rcc_translate_prefix_t {
    rcc_external_command_s cmd;
    unsigned long timeout;
    char from[3];
    char to[3];
    char text[1];
};
typedef struct rcc_translate_prefix_t rcc_translate_prefix_s;
typedef struct rcc_translate_prefix_t *rcc_translate_prefix;


struct rcc_translate_t {
    rcc_translate_prefix_s prefix;
    rcc_translate_prefix_s wprefix;
    size_t remaining;
    rcc_mutex mutex;
    rcc_mutex wmutex;
    int sock;

    unsigned char werror;
};
typedef struct rcc_translate_t rcc_translate_s;


int rccTranslateInit();
void rccTranslateFree();

#endif /* _RCC_TRANSLATE_H */
