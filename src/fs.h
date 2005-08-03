#ifndef _RCC_FS_H
#define _RCC_FS_H

int rccFS0(rcc_language_config config, const char *fspath, const char *filename, char **prefix, char **name);
int rccFS1(rcc_language_config config, const char *fspath, char **prefix, char **name);
char *rccFS2(rcc_language_config config, iconv_t icnv, const char *prefix, const char *name);
char *rccFS3(rcc_language_config config, rcc_class_id class_id, const char *prefix, const char *name);
char *rccFS5(rcc_context ctx, rcc_language_config config, rcc_class_id class_id, const char *utfstring);

#endif /* _RCC_FS_H */
