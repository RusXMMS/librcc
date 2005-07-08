#ifndef _LIBRCD_H
#define _LIBRCD_H

#ifdef __cplusplus
extern "C" {
#endif

enum rcd_russian_charset_t {
    RUSSIAN_CHARSET_WIN = 0,
    RUSSIAN_CHARSET_KOI,
    RUSSIAN_CHARSET_UTF8,
    RUSSIAN_CHARSET_ALT
};
typedef enum rcd_russian_charset_t rcd_russian_charset;

rcd_russian_charset (*rcdGetRussianCharset)(const char *buf, int len) = NULL;

#ifdef __cplusplus
}
#endif

#endif /* _LIBRCD_H */
