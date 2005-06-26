#ifndef _RCC_STRING_H
#define _RCC_STRING_H

rcc_string rccCreateString(rcc_language_id language_id, const char *buf, int len, int *rlen);
void rccStringFree(rcc_string str);

#endif /* _RCC_STRING_H */
