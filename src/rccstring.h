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

#ifdef HAVE_STRNLEN
# ifndef strnlen
int strnlen(const char *str, size_t size);
# endif /* !strnlen */
#else
int rccStrnlen(const char *str, size_t size);
#endif /* HAVE_STRNLEN */
int rccIsASCII(const char *str);

#endif /* _RCC_STRING_H */
