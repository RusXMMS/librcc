#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "rccstring.h"

rcc_string rccCreateString(rcc_language_id language_id, const char *buf, size_t len, size_t *rlen) {
    char *res;
    rcc_string_header header = {RCC_STRING_MAGIC, language_id};

    len = STRNLEN(buf, len);
        
    res = (char*)malloc(len+sizeof(rcc_string_header)+1);
    if (!res) return NULL;

    strncpy(res + sizeof(rcc_string_header), buf, len);
    res[sizeof(rcc_string_header) + len] = 0;

    memcpy(res, &header, sizeof(rcc_string_header));

    if (rlen) *rlen = len + sizeof(rcc_string_header);
    return (rcc_string)res;
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
    
    newlen = STRNLEN(str, len);
    if (newlen>sizeof(rcc_string_header)) {
	if ((len==newlen)&&(str[newlen-1])) return 0;
	newlen-=sizeof(rcc_string_header);
    } else return 0;

    if (((rcc_string_header*)str)->magic == RCC_STRING_MAGIC) return newlen;
    return 0;
}


rcc_language_id rccStringGetLanguage(const rcc_string str) {
    if (!str) return (rcc_language_id)-1;
    return ((rcc_string_header*)str)->language_id;
}

const char *rccStringGetString(const rcc_string str) {
    return (const char *)str + sizeof(rcc_string_header);
}

char *rccStringExtractString(const rcc_string str) {
    size_t len;
    char *res;
    
    len = rccStringCheck(str);
    if (!len) return NULL;
    
    res = (char*)malloc(len+1);
    if (!res) return NULL;

    strncpy(res, rccStringGetString(str), len);
    res[len] = 0;
    
    return res;    
}


const char *rccGetString(const char *str) {
    if (rccStringCheck(str)) return rccStringGetString((const rcc_string)str);
    return str;
}

const char *rccSizedGetString(const char *str, size_t len, size_t *rlen) {
    size_t newlen;
    
    newlen = rccStringSizedCheck(str, len);
    if (newlen) {
	if (rlen) *rlen = newlen;
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

