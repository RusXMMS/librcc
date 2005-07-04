#ifndef _RCC_STRING_H
#define _RCC_STRING_H

rcc_string rccCreateString(rcc_language_id language_id, const char *buf, int len, int *rlen);
void rccStringFree(rcc_string str);

rcc_language_id rccStringCheck(const rcc_string str);
const char *rccStringGet(const rcc_string str);
char *rccStringExtract(const rcc_string buf, int len, int *rlen);

int rccStringCmp(const rcc_string str1, const rcc_string str2);
int rccStringNCmp(const rcc_string str1, const rcc_string str2, size_t n);
int rccStringCaseCmp(const rcc_string str1, const rcc_string str2);
int rccStringNCaseCmp(const rcc_string str1, const rcc_string str2, size_t n);

#endif /* _RCC_STRING_H */
