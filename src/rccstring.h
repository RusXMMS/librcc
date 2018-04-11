/*
  LibRCC - this header defines internal string representation: UTF-8 encoding
  string + information on language.

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

#ifndef _RCC_STRING_H
#define _RCC_STRING_H

#define RCC_STRING_MAGIC 0xFF7F01FF

struct rcc_string_header_t {
    unsigned int magic;
    rcc_language_id language_id;
    char language[2];
};
typedef struct rcc_string_header_t rcc_string_header;


rcc_string rccCreateString(rcc_language_id language_id, const char *buf, size_t len);
void rccStringFree(rcc_string str);

int rccStringSetLang(rcc_string string, const char *sn);
int rccStringFixID(rcc_string string, rcc_context ctx);
int rccStringChangeID(rcc_string string, rcc_language_id language_id);

#ifndef HAVE_STRNLEN
int rccStrnlen(const char *str, size_t size);
#endif /* !HAVE_STRNLEN */
int rccIsASCII(const char *str);
size_t rccStringSizedGetChars(const char *str, size_t size);

#endif /* _RCC_STRING_H */
