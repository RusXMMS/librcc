#ifndef _RCC_ICONV_H
#define _RCC_ICONV_H

#include <iconv.h>

struct rcc_iconv_t {
    iconv_t icnv;
};
typedef struct rcc_iconv_t rcc_iconv_s;

int rccIConvGetError(rcc_iconv icnv);

size_t rccIConvInternal(rcc_context ctx, rcc_iconv icnv, const char *buf, size_t len);

/** 
  * Recodes chunk of data.
  *
  * @param icnv is recoding context
  * @param outbuf is preallocated output buffer
  * @param outsize is size of output buffer (striped string will be returned if buffer to small) 
  * @param buf is data for recoding
  * @param size is size of the data
  * @return number of recoded bytes in output buffer or -1 in the case of error
  */
size_t rccIConvRecode(rcc_iconv icnv, char *outbuf, size_t outsize, const char *buf, size_t size);

#endif /* _RCC_ICONV_H */
