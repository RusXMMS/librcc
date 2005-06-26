#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "rccstring.h"

rcc_string rccCreateString(rcc_language_id language_id, const char *buf, int len, int *rlen) {
    char *res;
    rcc_string_header header = {RCC_STRING_MAGIC, language_id};

    len = STRNLEN(buf, len);
        
    res = (char*)malloc(len+sizeof(rcc_string_header)+1);
    if (!res) return NULL;

    strncpy(res + sizeof(rcc_string_header), buf, len);
    res[sizeof(rcc_string_header) + 1 + len] = 0;

    memcpy(res, &header, sizeof(rcc_string_header));

    if (rlen) *rlen = len + sizeof(rcc_string_header);
    return (rcc_string)res;
}

void rccStringFree(rcc_string str) {
    if (str) free(str);
}

rcc_language_id rccStringCheck(const rcc_string str) {
    int len;
    rcc_string_header *header;

    len = strlen(str);
    
    if ((!str)||(len<=sizeof(unsigned int))||(*((unsigned int*)(str))!=RCC_STRING_MAGIC)) return 0;

    header = (rcc_string_header*)(str);
    return header->language_id;
}

const char *rccStringGet(const rcc_string str) {
    if (rccStringCheck(str)) return str + sizeof(rcc_string_header);
    return (const char *)str;
}

char *rccStringExtract(const rcc_string buf, int len, int *rlen) {
    char *res;
    
    len = STRNLEN(buf, len) - sizeof(rcc_string_header);
    if (len<0) return NULL;
    
    res = (char*)malloc(len+1);
    if (!res) return NULL;

    strncpy(res, buf + sizeof(rcc_string_header), len);
    res[len] = 0;
    
    if (rlen) *rlen = len;
    
    return res;    
}

int rccStringCmp(const rcc_string str1, const rcc_string str2) {
    return strcmp(rccStringGet(str1), rccStringGet(str2));
}

int rccStringNCmp(const rcc_string str1, const rcc_string str2, size_t n) {
    return strncmp(rccStringGet(str1), rccStringGet(str2), n);
}

int rccStringCaseCmp(const rcc_string str1, const rcc_string str2) {
    return strcasecmp(rccStringGet(str1), rccStringGet(str2));
}

int rccStringNCaseCmp(const rcc_string str1, const rcc_string str2, size_t n) {
    return strncasecmp(rccStringGet(str1), rccStringGet(str2), n);
}

