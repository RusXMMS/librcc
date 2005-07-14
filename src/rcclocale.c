#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "../config.h"

#ifdef HAVE_CODESET
# include <langinfo.h>
#endif

#include "rccconfig.h"

int rccLocaleGetClassByName(const char *locale) {
    if (!locale) return LC_CTYPE;

    if (!strcmp(locale, "LC_CTYPE")) return LC_CTYPE;
    if (!strcmp(locale, "LC_MESSAGES")) return LC_MESSAGES;
    if (!strcmp(locale, "LC_COLLATE")) return LC_COLLATE;
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
    } else {
	if (!strcasecmp(lv, "LANG")) l = getenv("LANG");
	else if (!strcasecmp(lv, "LANGUAGE")) l = getenv("LANGUAGE");
	else l = NULL;
    }
    if ((!l)||(strcmp(l,"C")==0)||(strcmp(l,"POSIX")==0)) return -1;

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
#ifdef HAVE_CODESET
	if (!l) l = nl_langinfo(CODESET);
#endif
	if (l) {
	    if (strlen(l)>=n) return -1;
	    strcpy(result, l);
	    return 0;
	}
    }

    if (locale_class >= 0) {
	l = setlocale(locale_class, NULL);
	if (!l) return -1;
    } else return -1;
    
    for (i=0;((l[i])&&(l[i]!='.')&&(l[i]!='_'));i++);
    if (i>=n) return -1;
    
    l = strrchr(l, '.');
    if (!l) return -1;

    for (i=0;((l[i])&&(l[i]!='@'));i++);
    if (i>=n) return -1;

    strncpy(result,l,i);
    result[i]=0;

    return 0;
}
