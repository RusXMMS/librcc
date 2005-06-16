#ifndef _RCC_FS_H
#define _RCC_FS_H

int rccFS0(const char *fspath, const char *filename, char **prefix, char **name);
int rccFS1(rcc_context *ctx, const char *fspath, char **prefix, char **name);
char *rccFS2(rcc_context *ctx, iconv_t icnv, const char *prefix, const char *name);
char *rccFS3(rcc_context *ctx, rcc_language_id language_id, rcc_class_id class_id, const char *prefix, const char *name);

#endif /* _RCC_FS_H */
