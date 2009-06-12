/*
  LibRCC - module handling internal string representation
  
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
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#include "internal.h"
#include "rccstring.h"

rcc_string rccCreateString(rcc_language_id language_id, const char *buf, size_t len) {
    char *res;
    rcc_string_header *header;

    if (!buf) return NULL;
    if (!len) len = strlen(buf);
        
    res = (char*)malloc(len+sizeof(rcc_string_header)+1);
    if (!res) return NULL;

    memcpy(res + sizeof(rcc_string_header), buf, len);
    res[sizeof(rcc_string_header) + len] = 0;

    memset(res, 0xFF, sizeof(rcc_string_header));
    header = (rcc_string_header*)res;
    header->magic = RCC_STRING_MAGIC;
    header->language_id = language_id;

    return (rcc_string)res;
}

int rccStringSetLang(rcc_string string, const char *sn) {
    if ((!string)||(!sn)||(strlen(sn)!=2)) return -1;
    memcpy(&((rcc_string_header*)string)->language,sn,2);
    return 0;
}

int rccStringFixID(rcc_string string, rcc_context ctx) {
    char lang[3];
    const char *curlang;
    rcc_language_config config;
    rcc_language_id language_id;
    rcc_string_header *header;

    if ((!string)||(!ctx)||(!rccStringCheck(string))) return -1;
    header = (rcc_string_header*)string;


    memcpy(lang, header->language, 2); lang[3] = 0;
    curlang = rccGetLanguageName(ctx, header->language_id);
    if ((curlang)&&(!strcasecmp(lang, curlang))) return 0;
    
    language_id = rccGetLanguageByName(ctx, lang);
    if ((language_id == (rcc_language_id)-1)||(language_id == 0)) return -1;
    config = rccGetConfig(ctx, language_id);
    if (!config) return -1;
    
    header->language_id = language_id;
    return 0;
}

int rccStringChangeID(rcc_string string, rcc_language_id language_id) {
    if ((!string)&&(language_id != (rcc_language_id)-1)) return -1;
    
//    printf("ChangingID %lu: %s\n", language_id, string);
    ((rcc_string_header*)string)->language_id = language_id;
    return 0;
}


void rccStringFree(rcc_string str) {
    if (str) free(str);
}

size_t rccStringCheck(const char *str) {
    size_t len;

    if (!str) return 0;
    
    len = strlen(str);
    if (len>sizeof(rcc_string_header)) {
	len-=sizeof(rcc_string_header);
        if (((rcc_string_header*)str)->magic == RCC_STRING_MAGIC) return len;
    }

    return 0;
}

size_t rccStringSizedCheck(const char *str, size_t len) {
    size_t newlen;

    if (!str) return 0;
    
    newlen = len?len:strlen(str);
    if (newlen>sizeof(rcc_string_header)) {
	if ((len==newlen)&&(!str[newlen-2])) return 0;
	newlen-=sizeof(rcc_string_header);
    } else return 0;
    if (((rcc_string_header*)str)->magic == RCC_STRING_MAGIC) return newlen;
    return 0;
}


rcc_language_id rccStringGetLanguage(rcc_const_string str) {
    if (!str) return (rcc_language_id)-1;
    return ((rcc_string_header*)str)->language_id;
}

const char *rccStringGetString(rcc_const_string str) {
    return (const char *)str + sizeof(rcc_string_header);
}

char *rccStringExtractString(rcc_const_string str) {
    size_t len;
    char *res;
    
    len = rccStringCheck(str);
    if (!len) return NULL;
    
    res = (char*)malloc(len+1);
    if (!res) return NULL;

    memcpy(res, rccStringGetString(str), len);
    res[len] = 0;
    
    return res;    
}


const char *rccGetString(const char *str) {
    if (rccStringCheck(str)) return rccStringGetString((const rcc_string)str);
    return str;
}

const char *rccSizedGetString(const char *str, size_t len) {
    size_t newlen;
    
    newlen = rccStringSizedCheck(str, len);
    if (newlen) {
	return rccStringGetString((const rcc_string)str);
    }
    
    return (const char *)str;
}


int rccStringCmp(const char *str1, const char *str2) {
    return strcmp(rccGetString(str1), rccGetString(str2));
}

int rccStringNCmp(const char *str1, const char *str2, size_t n) {
    return strncmp(rccGetString(str1), rccGetString(str2), n);
}

int rccStringCaseCmp(const char *str1, const char *str2) {
    return strcasecmp(rccGetString(str1), rccGetString(str2));
}

int rccStringNCaseCmp(const char *str1, const char *str2, size_t n) {
    return strncasecmp(rccGetString(str1), rccGetString(str2), n);
}

#ifndef HAVE_STRNLEN
int rccStrnlen(const char *str, size_t size) {
    unsigned int i;
    for (i=0;((i<size)&&(str[i]));i++);
    return i;
}
#endif /* HAVE_STRNLEN */


int rccIsASCII(const char *str) {
    unsigned int i;
    
    for (i=0;str[i];i++)
	if ((unsigned char)str[i]>0x7F) return 0;
    return 1;
}

size_t rccStringSizedGetChars(const char *str, size_t size) {
    size_t i, skip = 0, chars = 0;
    const unsigned char *tmp;

    tmp = (unsigned char*)rccGetString(str);
    
    for (i=0;(size?(size-i):tmp[i]);i++) {
	if (skip) {
	    skip--;
	    continue;
	}
	
	if (tmp[i]<0x80) skip = 0;
	else if ((tmp[i]>0xBF)&&(tmp[i]<0xE0)) skip = 1;
	else if ((tmp[i]>0xDF)&&(tmp[i]<0xF0)) skip = 2;
	else if ((tmp[i]>0xEF)&&(tmp[i]<0xF8)) skip = 3;
	else skip = 4;
	chars++;
    }
    
    return chars;
}
