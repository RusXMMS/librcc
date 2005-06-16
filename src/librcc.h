#ifndef LIBRCC_H
#define LIBRCC_H

#include <iconv.h>

#define RCC_MAX_CHARSETS 16
#define RCC_MAX_ENGINES 5
#define RCC_MAX_LANGUAGES 64
#define RCC_MAX_CLASSES 16

#define RCC_MAX_ERRORS 3

#define RCC_MAX_CHARSET_CHARS 16
#define RCC_MAX_LANGUAGE_CHARS 16
#define RCC_MAX_VARIABLE_CHARS 16

#define RCC_MAX_STRING_CHARS 1024

#define RCC_STRING_MAGIC 0xFF7F01FF
/*
    class = charset class
    engine = auto engine
    selected - which is selected
    current - resolves default values
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int rcc_init_flags;
#define RCC_DEFAULT_CONFIGURATION 1

typedef int rcc_option_value;
typedef enum rcc_option_t {
    RCC_LEARNING_MODE = 0,
    RCC_AUTODETECT_FS_TITLES,
    RCC_AUTODETECT_FS_NAMES,
    RCC_USE_HEADERS,
    RCC_MAX_OPTIONS
} rcc_option;

typedef enum rcc_class_type_t {
    RCC_CLASS_INVALID = 0,
    RCC_CLASS_STANDARD,
    RCC_CLASS_FS
} rcc_class_type;


struct rcc_string_header_t {
    unsigned int magic;
    rcc_language_id language_id;
};
typedef struct rcc_string_header_t rcc_string_header;

typedef char *rcc_string;
typedef char rcc_language_id;
typedef char rcc_charset_id;
typedef char rcc_engine_id;
typedef int rcc_class_id;

typedef struct rcc_context_t rcc_context;
typedef struct rcc_engine_context_t rcc_engine_ctx;
typedef const struct rcc_class_t rcc_class;
typedef struct rcc_language_t rcc_language;
typedef struct rcc_engine_t rcc_engine;
typedef const char *rcc_charset;

typedef struct rcc_language_config_t rcc_language_config;
typedef const struct rcc_language_alias_t rcc_language_alias;

typedef void *rcc_engine_internal;
typedef rcc_engine_internal (*rcc_engine_init_function)(rcc_engine_context *ctx);
typedef rcc_charset_id (*rcc_engine_function)(rcc_engine_context ctx, char *buf, int len);
typedef void (rcc_engine_free_function)(rcc_engine_context ctx);

typedef rcc_charset rcc_charset_list[RCC_MAX_CHARSETS+1];

struct rcc_engine_t {
    const char *title;
    rcc_engine_init_function init_func;
    rcc_engine_free_function free_func;
    rcc_engine_function func;
    rcc_charset_list charsets;
};
typedef rcc_engine *rcc_engine_ptr;
typedef rcc_engine_ptr rcc_engine_list[RCC_MAX_ENGINES+1];

struct rcc_language_t {
    const char *sn;
    const char *name;
    rcc_charset_list charsets;
    rcc_engine_list engines;
};
typedef rcc_language *rcc_language_ptr;
typedef rcc_language_ptr rcc_language_list[RCC_MAX_LANGUAGES+1];

struct rcc_language_alias_t {
    const char *alias;
    const char *lang;
};

struct rcc_class_t {
    const char *name;
    const char *defvalue; /* locale variable name or parrent name */
    const rcc_class_type class_type;
};
typedef rcc_class *rcc_class_ptr;
typedef rcc_class_ptr rcc_class_list[RCC_MAX_CLASSES+1];

struct rcc_language_config_t {
    rcc_context *ctx;
    rcc_language *language;
    
    rcc_engine_id engine;
    rcc_charset_id *charset;
    rcc_option_value options[RCC_MAX_OPTIONS];
};

struct rcc_engine_context_t {
    rcc_context *ctx;
    rcc_language *language;
    
    rcc_engine_function func;
    rcc_engine_free_function free_func;

    rcc_engine_internal internal;
};

struct rcc_context_t {
    char locale_variable[RCC_MAX_VARIABLE_CHARS+1];
    
    unsigned int max_languages;
    unsigned int n_languages;
    rcc_language_ptr *languages;
    rcc_language_config *configs;
    
    unsigned int max_classes;
    unsigned int n_classes;
    rcc_class_ptr *classes;

    rcc_engine_ctx engine_ctx;

    iconv_t *iconv_from;
    iconv_t *iconv_to;
    iconv_t iconv_auto[RCC_MAX_CHARSETS];

    char tmpbuffer[RCC_MAX_STRING_CHARS+sizeof(rcc_string_footer)+1];
    iconv_t fsiconv;
    
    unsigned char configure;
    rcc_language_config *current_config;
    rcc_language_id current_language;
};

int rccInit();
void rccFree();

rcc_context rccInitContext(rcc_init_flags flags, unsigned int max_languages, unsigned int max_classes, const char *locale);
void rccFreeContext(rcc_context *ctx);

rcc_language_id rccRegisterLanguage(rcc_context *ctx, rcc_language *language);
rcc_charset_id rccLanguageRegisterCharset(rcc_language *language, rcc_charset charset);
rcc_engine_id rccLanguageRegisterEngine(rcc_language *language, rcc_engine *engine);
rcc_class_id rccRegisterClass(rcc_context *ctx, rcc_class *cl);

rcc_class_type rccGetClassType(rcc_context *ctx, rcc_class_id class_id);

int rccConfigure(rcc_context *ctx);
char *rccCreateResult(rcc_context *ctx, int len, int *rlen);

/* lng.c */
const char *rccGetLanguageName(rcc_context *ctx, rcc_language_id language_id);
rcc_language_id rccGetLanguageByName(rcc_context *ctx, const char *name);
rcc_language_id rccGetRealLanguage(rcc_context *ctx, rcc_language_id language_id);
const char *rccGetRealLanguageName(rcc_context *ctx, rcc_language_id language_id);
rcc_language_id rccGetSelectedLanguage(rcc_context *ctx);
const char *rccGetSelectedLanguageName(rcc_context *ctx);
rcc_language_id rccGetCurrentLanguage(rcc_context *ctx);
const char *rccGetCurrentLanguageName(rcc_context *ctx);

int rccSetLanguage(rcc_context *ctx, rcc_language_id language_id);
int rccSetLanguageByName(rcc_context *ctx, const char *name);

/* lngconfig.c */
int rccConfigInit(rcc_language_config *config, rcc_context *ctx);
int rccConfigFree(rcc_language_config *config);

const char *rccConfigGetEngineName(rcc_language_config config, rcc_engine_id engine_id);
const char *rccConfigGetCharsetName(rcc_language_config config, rcc_charset_id charset_id);
const char *rccConfigGetAutoCharsetName(rcc_language_config config, rcc_charset_id charset_id);
rcc_engine_id rccConfigGetEngineByName(rcc_language_config *config, const char *name);
rcc_charset_id rccConfigGetCharsetByName(rcc_language_config *config, const char *name);
rcc_charset_id rccConfigGetAutoCharsetByName(rcc_language_config *config, const char *name);

rcc_language_config *rccGetConfig(rcc_context *ctx, rcc_language_id language_id);
rcc_language_config *rccGetConfigByName(rcc_context *ctx, const char *name);
rcc_language_config *rccGetCurrentConfig(rcc_context *ctx);

rcc_engine_id rccConfigGetSelectedEngine(rcc_language_config config);
const char *rccConfigGetSelectedEngineName(rcc_language_config config);
rcc_engine_id rccConfigGetCurrentEngine(rcc_language_config config);
const char *rccConfigGetCurrentEngineName(rcc_language_config config);
rcc_charset_id rccConfigGetSelectedCharset(rcc_language_config config, rcc_class_id class_id);
const char *rccConfigGetSelectedCharsetName(rcc_language_config config, rcc_class_id class_id);
rcc_charset_id rccConfigGetCurrentCharset(rcc_language_config config, rcc_class_id class_id);
const char *rccConfigGetCurrentCharsetName(rcc_language_config config, rcc_class_id class_id);
rcc_option_value rccConfigGetOption(rcc_language_config config, rcc_option option);

int rccConfigSetEngine(rcc_language_config *config, rcc_engine_id engine_id);
int rccConfigSetCharset(rcc_language_config *config, rcc_class_id class_id, rcc_charset_id charset_id);
int rccConfigSetEngineByName(rcc_language_config *config, const char *name);
int rccConfigSetCharsetByName(rcc_language_config *config, rcc_class_id class_id, const char *name);
int rccConfigSetOption(rcc_language_config *config, rcc_option option, rcc_option_value value);

rcc_charset_id rccConfigGetLocaleCharset(rcc_language_config *config, const char *locale_variable);

/* curconfig.c */
#define rccGetEngineName(ctx, engine_id) rccConfigGetEngineName(ctx->current_config, engine_id)
#define rccGetCharsetName(ctx, charset_id) rccConfigGetCharsetName(ctx->current_config, charset_id)
#define rccGetAutoCharsetName(ctx, charset_id) rccConfGetAutoCharsetName(ctx->current_config, charset_id)
#define rccGetEngineByName(ctx, name) rccConfigGetEngineByName(ctx->current_config, name)
#define rccGetCharsetByName(ctx, name) rccConfigGetCharsetByName(ctx->current_config, name)
#define rccGetAutoCharsetByName(ctx, name) rccConfigGetAutoCharsetByName(ctx->current_config, name)

#define rccGetSelectedEngine(ctx) rccConfigGetSelectedEngine(ctx->current_config)
#define rccGetSelectedEngineName(ctx) rccConfigGetSelectedEngineName(ctx->current_config)
#define rccGetCurrentEngine(ctx) rccConfigGetCurrentEngine(ctx->current_config)
#define rccGetCurrentEngineName(ctx) rccConfigGetCurrentEngineName(ctx->current_config)
#define rccGetSelectedCharset(ctx,class_id) rccConfigGetSelectedCharset(ctx->current_config, class_id)
#define rccGetSelectedCharsetName(ctx,class_id) rccConfigGetSelectedCharsetName(ctx->current_config, class_id)
#define rccGetCurrentCharset(ctx,class_id) rccConfigGetCurrentCharset(ctx->current_config, class_id)
#define rccGetCurrentCharsetName(ctx,class_id) rccConfigGetCurrentCharsetName(ctx->current_config, class_id)
#define rccGetOption(ctx, option) rccConfigGetOption(ctx->current_config, option)

#define rccSetEngine(ctx, engine_id) rccConfigSetEngine(ctx->current_config, engine_id)
#define rccSetCharset(ctx, class_id, charset_id) rccConfigSetCharset(ctx->current_config, class_id, charset_id)
#define rccSetOption(ctx,option,value) rccConfigSetOption(ctx->current_config, option, value)
#define rccSetEngineByName(ctx, name) rccConfigSetEngineByName(ctx->current_config, name)
#define rccSetCharsetByName(ctx, class_id, name) rccConfigSetCharsetByName(ctx->current_config, class_id, name)

#define rccGetLocaleCharset(ctx, locale_variable) rccConfigGetLocaleCharset(ctx->current_config, locale_variable)

/* recode.c */
char *rccFrom(rcc_context *ctx, rcc_class_id class_id, char *buf, int len, int *rlen);
char *rccTo(rcc_context *ctx, rcc_class_id class_id, char *buf, int len, int *rlen);
char *rccRecode(rcc_context *ctx, rcc_class_id from, rcc_class_id to, char *buf, int len, int *rlen);
char *rccFS(rcc_context *ctx, char *fspath, char *path, char *filename, int len, int *rlen);

/* string.c */
rcc_string rccStringInit(rcc_language_id language_id, const char *buf, int len, int *rlen);
void rccStringFree(rcc_string str);

rcc_language_id rccStringCheck(const rcc_string str);
const char *rccStringGet(const rcc_string str);
char *rccStringExtract(const rcc_string buf, int len, int *rlen);

char *rccStringCmp(const rcc_string str1, const rcc_string str2);
char *rccStringNCmp(const rcc_string str1, const rcc_string str2, size_t n);
char *rccStringCaseCmp(const rcc_string str1, const rcc_string str2);
char *rccStringNCaseCmp(const rcc_string str1, const rcc_string str2, size_t n);

/* xml.c */
int rccSave(rcc_context *ctx);
int rccLoad(rcc_context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* LIBRCC_H */
