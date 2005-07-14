#ifndef _RCC_LOCALE_H
#define _RCC_LOCALE_H

int rccLocaleGetClassByName(const char *locale);
int rccLocaleGetLanguage(char *result, const char *lv, unsigned int n);
int rccLocaleGetCharset(char *result, const char *lv, unsigned int n);

#endif /* _RCC_LOCALE_H */
