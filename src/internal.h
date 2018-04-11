/*
  LibRCC - various definitions used internally

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

#ifndef _RCC_INTERNAL_H
#define _RCC_INTERNAL_H

#ifndef LIBRCC_DATA_DIR
# define LIBRCC_DATA_DIR "/usr/lib/rcc"
#endif /* LIBRCC_DATA_DIR */

#define RCC_MAX_LANGUAGE_PARENTS 4
#define RCC_MAX_RELATIONS RCC_MAX_LANGUAGES

#ifdef HAVE_STRNLEN
#define STRNLEN(str,n) (n?strnlen(str,n):strlen(str))
#else
#define STRNLEN(str,n) (n?rccStrnlen(str,n):strlen(str))
#endif /* !strnlen */

#define RCC_MAX_PLUGINS 32
#define RCC_MAX_STRING_CHARS 1024
#define RCC_MAX_PREFIX_CHARS 32
#define RCC_MIN_DB4_CHARS 3

#include "librcc.h"
#include "recode.h"
#include "engine.h"
#include "lngconfig.h"
#include "rccstring.h"
#include "rccdb4.h"
#include "rcciconv.h"
#include "rccstring.h"
#include "rccmutex.h"
#include "rcclocale.h"

#define RCC_MAX_ADDITION_CHARSETS	8
#define RCC_MAX_DISABLED_CHARSETS	64


typedef rcc_language_id rcc_language_parent_list[RCC_MAX_LANGUAGE_PARENTS];

struct rcc_language_internal_t {
    rcc_language language;
    rcc_language_id parents[RCC_MAX_LANGUAGE_PARENTS + 1];
    unsigned char latin;
};
typedef struct rcc_language_internal_t rcc_language_internal;
typedef rcc_language_internal *rcc_language_internal_ptr;

struct rcc_class_internal_t {
    rcc_class cl;
    rcc_charset *additional;
    rcc_charset *disabled;
};
typedef struct rcc_class_internal_t rcc_class_internal;

struct rcc_context_t {
    char locale_variable[RCC_MAX_VARIABLE_CHARS+1];
    
    rcc_language_alias_list aliases;

    rcc_option_value options[RCC_MAX_OPTIONS];
    unsigned char default_options[RCC_MAX_OPTIONS];
     
    unsigned int max_languages;
    unsigned int n_languages;
    rcc_language_internal *ilang;
    rcc_language_ptr *languages;
    rcc_language_config configs;
    
    unsigned int max_classes;
    unsigned int n_classes;
    rcc_class_internal *iclass;
    rcc_class_ptr *classes;

    rcc_iconv *iconv_from;
    rcc_iconv iconv_auto[RCC_MAX_CHARSETS];

    rcc_iconv fsiconv;

    char tmpbuffer[RCC_MAX_STRING_CHARS+sizeof(rcc_string_header)+9];
    char lastprefix[RCC_MAX_PREFIX_CHARS+1];
    
    unsigned char configure;
    rcc_language_config current_config;
    rcc_language_id current_language;
    
    rcc_language_id default_language;

    db4_context db4ctx;
    
    rcc_mutex mutex;
    unsigned int configuration_lock;
};
typedef struct rcc_context_t rcc_context_s;

int rccLock();
void rccUnLock();

int rccConfigure(rcc_context ctx);
char *rccCreateResult(rcc_context ctx, size_t len);

extern rcc_context rcc_default_ctx;

#endif /* _RCC_INTERNAL_H */
