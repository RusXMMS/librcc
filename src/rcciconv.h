#ifndef _RCC_ICONV_H
#define _RCC_ICONV_H

struct rcc_iconv_t {
    iconv_t icnv;
};
typedef struct rcc_iconv_t rcc_iconv_s;

size_t rccIConv(rcc_context ctx, iconv_t icnv, const char *buf, size_t len);

#endif /* _RCC_ICONV_H */
