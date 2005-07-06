#ifndef _RCC_STRING_H
#define _RCC_STRING_H

#define RCC_STRING_MAGIC 0xFF7F01FF

struct rcc_string_header_t {
    unsigned int magic;
    rcc_language_id language_id;
};
typedef struct rcc_string_header_t rcc_string_header;


rcc_string rccCreateString(rcc_language_id language_id, const char *buf, size_t len, size_t *rlen);
void rccStringFree(rcc_string str);

#endif /* _RCC_STRING_H */
