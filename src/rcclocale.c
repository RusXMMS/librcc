/*
  LibRCC - module responsible for extracting locale information

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
#include <locale.h>

#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */


#include "../config.h"

#ifdef HAVE_LIBCHARSET
# include <libcharset.h>
#endif /* HAVE_LIBCHARSET */
#ifdef HAVE_CODESET
# include <langinfo.h>
#endif


#include "rccconfig.h"

int rccLocaleGetClassByName(const char *locale) {
#ifdef LC_CTYPE
    if (!locale) return LC_CTYPE;
    
    if (!strcmp(locale, "LC_CTYPE")) return LC_CTYPE;
#endif /* LC_CTYPE */
#ifdef LC_MESSAGES
    if (!strcmp(locale, "LC_MESSAGES")) return LC_MESSAGES;
#endif /* LC_MESSAGES */
#ifdef LC_COLLATE
    if (!strcmp(locale, "LC_COLLATE")) return LC_COLLATE;
#endif /* LC_COLLATE */
/*
    if (!strcmp(locale, "LC_ALL")) return LC_ALL;
    if (!strcmp(locale, "LC_NUMERIC")) return LC_NUMERIC;
    if (!strcmp(locale, "LC_MONETARY")) return LC_MONETARY;
    if (!strcmp(locale, "LC_PAPER")) return LC_PAPER;
    if (!strcmp(locale, "LC_NAME")) return LC_NAME;
    if (!strcmp(locale, "LC_ADDRESS")) return LC_ADDRESS;
    if (!strcmp(locale, "LC_TELEPHONE")) return LC_TELEPHONE;
    if (!strcmp(locale, "LC_MEASURMENT")) return LC_MEASURMENT;
    if (!strcmp(locale, "LC_IDENTIFICATION")) return LC_IDENTIFICATION;
*/
    return -1;
}

int rccLocaleGetLanguage(char *result, const char *lv, unsigned int n) {
    unsigned int i, j;
    int locale_class;
    const char *l;
    
    if ((!result)||(!n)) return -1;

    locale_class = rccLocaleGetClassByName(lv);
    if (locale_class >= 0) {
	l = setlocale(locale_class, NULL);
	if (!strcasecmp(l,"C")) {
	    setlocale(LC_ALL, "");
	    l = setlocale(locale_class, NULL);
	}
    } else {
	if (!strcasecmp(lv, "LANG")) l = getenv("LANG");
	else if (!strcasecmp(lv, "LANGUAGE")) l = getenv("LANGUAGE");
	else l = NULL;
    }
    if ((!l)||(!strcmp(l,"C"))||(!strcmp(l,"POSIX"))) return -1;

    for (i=0;((l[i])&&(l[i]!='.')&&(l[i]!=':'));i++);

    for (j=0;rcc_default_aliases[j].alias;j++) 
	if (strncmp(l,rcc_default_aliases[j].alias,i)==0) {
	    l = rcc_default_aliases[j].alias;
	    break;
	}

    for (i=0;((l[i])&&(l[i]!='.')&&(l[i]!='_'));i++);
    if (i>=n) return -1;

    strncpy(result,l,i);
    result[i]=0;

    return 0;
}

int rccLocaleGetCharset(char *result, const char *lv, unsigned int n) {
    unsigned int i;
    int locale_class;
    char *l;

    if ((!result)||(!n)) return -1;

    locale_class = rccLocaleGetClassByName(lv);
    
    if (locale_class == LC_CTYPE) {
	l = getenv("CHARSET");
#ifdef HAVE_LIBCHARSET
	if (!l) l = (char*)locale_charset();
#endif /* HAVE_LIBCHARSET */
#ifdef HAVE_CODESET
	if (!l) l = nl_langinfo(CODESET);
#endif /* HAVE_CODESET */
	if (l) {
	    if (strlen(l)>=n) return -1;
	    strcpy(result, l);
	    return 0;
	}
    }

    if (locale_class >= 0) {
	l = setlocale(locale_class, NULL);
	if (!strcasecmp(l,"C")) {
	    setlocale(LC_ALL, "");
	    l = setlocale(locale_class, NULL);
	}
	if (!l) return -1;
    } else return -1;
    
    for (i=0;((l[i])&&(l[i]!='.')&&(l[i]!='_'));i++);
    if (i>=n) return -1;
    
    l = strrchr(l, '.');
    if (!l) return -1;

    for (i=0;((l[i])&&(l[i]!='@'));i++);
    if (i>=n) return -1;

    strncpy(result,l+1,i-1);
    result[i]=0;

    return 0;
}
