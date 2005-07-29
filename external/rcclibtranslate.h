#ifndef _RCC_LIBTRANSLATE_H
#define _RCC_LIBTRANSLATE_H

int rccLibTranslateInit(const char *rcc_home_dir);
void rccLibTranslateFree();

void *rccLibTranslate(void *info);

#endif /* _RCC_LIBTRANSLATE_H */
