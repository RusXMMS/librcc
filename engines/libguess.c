/*
  LibRCC - LibGUESS Engine

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

#include <stdio.h>
#include <string.h>

#include <librcc.h>
#include <libguess.h>

#define UTF8_ID 0
#define UTF16_ID 1

typedef const char *(*guess_function)(const char *buf, int buflen);

struct rcc_guess_engine_t {
    struct rcc_engine_t engine;
    guess_function func;
};
typedef struct rcc_guess_engine_t rcc_guess_engine;

rcc_autocharset_id guessDetect(rcc_engine_context ctx, const char *buf, int len) {
    const char *res;
    rcc_guess_engine *info;

    if (!buf) return (rcc_autocharset_id)-1;
    
    info = (rcc_guess_engine*)rccEngineGetInfo(ctx);
    if (info) {
	if (info->func) res = info->func(buf, len?len:strlen(buf));
	else {
	    if (!len) len = strlen(buf);
	    res = guess_cn(buf, len);
	    if (!res) res = guess_tw(buf, len);
	}
    } else 
	res = NULL;
	
    if (!res) return (rcc_autocharset_id)-1;
    return rccEngineGetAutoCharsetByName(ctx, res);
}

    
struct rcc_guess_engine_t guessJPEngine = {
 {
    "LibGUESS",
    NULL,		/* Constructor */
    NULL, 		/* Destructor */
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-JP", "EUC-JP", "SJIS", NULL}
 },
 &guess_jp
};

struct rcc_guess_engine_t guessZHEngine = {
 {
    "LibGUESS",
    NULL,		/* Constructor */
    NULL, 		/* Destructor */
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-CN", "GB2312", "GB18030", "BIG5", NULL}
 },
 NULL
};

/*
struct rcc_guess_engine_t guessCNEngine = {
 {
    "LibGUESS",
    NULL,
    NULL,
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-CN", "GB2312", "GB18030", NULL}
 },
 &guess_cn
};

struct rcc_guess_engine_t guessTWEngine = {
 {
    "LibGUESS",
    NULL,
    NULL,
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-TW", "BIG5", NULL}
 },
 &guess_tw
};
*/

struct rcc_guess_engine_t guessKREngine = {
 {
    "LibGUESS",
    NULL,		/* Constructor */
    NULL, 		/* Destructor */
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-KR", "EUC-KR", "JOHAB", NULL}
 },
 &guess_kr
};
        
    
rcc_engine *rccGetInfo(const char *lang) {
    if (!strcmp(lang, "zh")) return (rcc_engine*)&guessZHEngine;
    if (!strcmp(lang, "ja")) return (rcc_engine*)&guessJPEngine;
    if (!strcmp(lang, "ko")) return (rcc_engine*)&guessKREngine;
    return NULL;
}
