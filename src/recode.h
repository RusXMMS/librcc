#ifndef _RCC_RECODE_H
#define _RCC_RECODE_H

#define RCC_STRING_MAGIC 0xFF7F01FF

struct rcc_string_header_t {
    unsigned int magic;
    rcc_language_id language_id;
};
typedef struct rcc_string_header_t rcc_string_header;


#endif /* _RCC_RECODE_H */
