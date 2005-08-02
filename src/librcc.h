#ifndef _LIBRCC_H
#define _LIBRCC_H

#include <stdlib.h>

/*******************************************************************************
***************************** Global Defines ***********************************
*******************************************************************************/

#define RCC_MAX_CHARSETS 16
#define RCC_MAX_ENGINES 5
#define RCC_MAX_LANGUAGES 64
#define RCC_MAX_ALIASES 64
#define RCC_MAX_CLASSES 16

#define RCC_MAX_ERRORS 3

#define RCC_MAX_CHARSET_CHARS 16
#define RCC_MAX_LANGUAGE_CHARS 16
#define RCC_MAX_VARIABLE_CHARS 16


/* ID's */
/**
  * Language ID. 
  *    -  0 is default language
  *    - -1 is error
  *    -  1 usually represents "LibRCC off" language
  *    - >1 is some language
  */
typedef unsigned char rcc_language_id;
/**
  * Alias ID
  */
typedef unsigned char rcc_alias_id;
/**
  * Charset ID.
  *	-  0 is default charset
  *	- -1 is error
  *	- >0 is some charset
  */
typedef unsigned char rcc_charset_id;
/**
  * Autocharset ID.
  *	- -1 is error
  *	- >0 is some encoding
  */
typedef unsigned char rcc_autocharset_id;
/**
  * Engine ID.
  *	- -1 is non configured (first available will be used if any)
  *	-  0 autodetection is switched off
  *	- >0 is some auto-engine
  */
typedef unsigned char rcc_engine_id;
/**
  * Class ID.
  */
typedef int rcc_class_id;

/* Opaque Pointer's */
typedef struct rcc_context_t *rcc_context; /**< Current Working Context */
/** 
  * Encoding Detection Engine Context. Containes considered information about
  * engine, which can be extracted using API functions.
  * @see rccEngineGetInternal
  * @see rccEngineGetLanguage
  * @see rccEngineGetRccContext
  */
typedef struct rcc_engine_context_t *rcc_engine_context; 
/**
  * Current Language Configuration:
  * Information about current 'class encodings', and selected 'Encoding Detection
  * Engine'
  */
typedef struct rcc_language_config_t *rcc_language_config; 
typedef const struct rcc_class_t *rcc_class_ptr; 

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Library Initialization function. Should be called prior to all
 * any library manipulation.
 */
int rccInit();

/**
 * Library Cleanup function.
 */
void rccFree();

/*******************************************************************************
**************************** Initialization ************************************
*******************************************************************************/
/** 
  * RCC context initialization flags
  */
typedef unsigned int rcc_init_flags;

/** 
  * Do not load default language configuration 
  */
#define RCC_FLAG_NO_DEFAULT_CONFIGURATION 1 

/**
 * Initialize working context.
 *
 * @param locale_variable is variable to get locale from (Default: LC_CTYPE).
 * @param max_languages is maximal number of languages supported by context. (Default: detect)
 * @param max_classes is maximal number of classes (Default: detect)
 * @param defclasses is list of encoding classes (Default: will add later)
 * @param flags is option flag (Default: nothing)
 * @see RCC_FLAG_NO_DEFAULT_CONFIGURATION
 * @return working context or NULL in case of error
 */
rcc_context rccCreateContext(const char *locale_variable, unsigned int max_languages, unsigned int max_classes, rcc_class_ptr defclasses, rcc_init_flags flags);
/**
 * Initialize default working context (used then no context supplied). Previously
 * opened default context will be freed.
 *
 * @param locale_variable is variable to get locale from (Default: LC_CTYPE).
 * @param max_languages is maximal number of languages supported by context. (Default: detect)
 * @param max_classes is maximal number of classes (Default: detect)
 * @param defclasses is list of encoding classes (Default: will add later)
 * @param flags is option flag (Default: nothing)
 * @see RCC_FLAG_NO_DEFAULT_CONFIGURATION
 * @return non-zero value in case of error
 */
int rccInitDefaultContext(const char *locale_variable, unsigned int max_languages, unsigned int max_classes, rcc_class_ptr defclasses, rcc_init_flags flags);

/**
 * Free all memory used by working context and destroy it. 
 *
 * @param ctx is working context to be destroyed.
 */
void rccFreeContext(rcc_context ctx);


/** 
  * Berkeley DB initialization flags
  */
typedef unsigned int rcc_db4_flags;

/**
 * Enables Berkeley DB recodings caching for specified working context.
 *
 * @param ctx is working context
 * @param name is database name (can be shared between different applications)
 * @param flags are reserved for future.
 * @return non-zero value in case of error
 */
int rccInitDb4(rcc_context ctx, const char *name, rcc_db4_flags flags);

int rccLockConfiguration(rcc_context ctx, unsigned int lock_code);
int rccUnlockConfiguration(rcc_context ctx, unsigned int lock_code);

/*******************************************************************************
******************* Altering Language Configuaration ***************************
*******************************************************************************/
/**
  * Encoding name.
  */
typedef const char *rcc_charset;
/**
  * List of Encoding names
  */
typedef rcc_charset rcc_charset_list[RCC_MAX_CHARSETS+1];

/* Engines */
/**
  * Engine internal data
  */
typedef void *rcc_engine_internal;
/**
  * Engine constructor function
  * @param ctx is engine context
  * @see rccEngineGetInternal
  * @see rccEngineGetLanguage
  * @see rccEngineGetRccContext
  * @return pointer on desired internal data to be stored in engine_context.
  */
typedef rcc_engine_internal (*rcc_engine_init_function)(rcc_engine_context ctx);
/**
  * Engine encoding detection function.
  * @param ctx is engine context
  * @param buf is string encoded in unknow encoding
  * @param len is exact size of string or 0 (size will be detected with strlen)
  * @return the #rcc_autocharset_id of the detected encoding or -1 in case of error
  */
typedef rcc_autocharset_id (*rcc_engine_function)(rcc_engine_context ctx, const char *buf, int len);
/**
  * Engine destructor function
  */
typedef void (*rcc_engine_free_function)(rcc_engine_context ctx);

/**
  * Encoding detection engine description. Init and Free functions can be omited.
  * 'func' should analyze string and return position in the encodings list
  * coresponding to string encoding.
  */
struct rcc_engine_t {
    const char *title; /**< Short title*/
    rcc_engine_init_function init_func; /**< Constructor function */
    rcc_engine_free_function free_func; /**< Destructor function */
    rcc_engine_function func; /**< Function performing encoding detection */
    rcc_charset_list charsets; /**< List of supported encodings */
};
typedef struct rcc_engine_t rcc_engine;
typedef rcc_engine *rcc_engine_ptr;
typedef rcc_engine_ptr rcc_engine_list[RCC_MAX_ENGINES+1];

/**
  * Language description.
  */
struct rcc_language_t {
    const char *sn; /**< Language ISO-639-1 (2 symbol) name */
    rcc_charset_list charsets; /**< List of language encodings */
    rcc_engine_list engines; /**< List of encoding detection engines supported by language */
};
typedef struct rcc_language_t rcc_language;
typedef rcc_language *rcc_language_ptr;
typedef rcc_language_ptr rcc_language_list[RCC_MAX_LANGUAGES+1];

/** 
  * Language Aliases.
  * For example (ru_UA = uk, cs_SK = sk )
  */
struct rcc_language_alias_t {
    const char *alias; /**< Long locale name */
    const char *lang; /* Coresponded language ISO-639-1 name */
};
typedef struct rcc_language_alias_t rcc_language_alias;
typedef rcc_language_alias *rcc_language_alias_ptr;
typedef rcc_language_alias_ptr rcc_language_alias_list[RCC_MAX_ALIASES+1];

/**
  * Register new language in supplied working context
  * @param ctx is working context ( or default one if NULL supplied )
  * @param language is pointer on language description (shouldn't be freed before library deinitialization).
  * @return registered language id  or -1 in case of a error.
  */
rcc_language_id rccRegisterLanguage(rcc_context ctx, rcc_language *language);
/**
  * Register new encoding belonging to language in supplied working context
  * @param language is language charset should be added to ( or default one if NULL supplied )
  * @param charset is pointer on charset name (shouldn't be freed before library deinitialization).
  * @return registered charset id  or -1 in case of a error.
  */
rcc_charset_id rccLanguageRegisterCharset(rcc_language *language, rcc_charset charset);
/**
  * Register new Engine in supplied working context
  * @param language is language charset should be added to ( or default one if NULL supplied )
  * @param engine is pointer on engine description (shouldn't be freed before library deinitialization).
  * @return registered engine id  or -1 in case of a error.
  */
rcc_engine_id rccLanguageRegisterEngine(rcc_language *language, rcc_engine *engine);
/**
  * Register new language alias in supplied working context
  * @param ctx is working context ( or default one if NULL supplied )
  * @param alias is pointer on alias description (shouldn't be freed before library deinitialization).
  * @return registered alias id  or -1 in case of a error.
  */
rcc_alias_id rccRegisterLanguageAlias(rcc_context ctx, rcc_language_alias *alias);

/*******************************************************************************
************************ Altering Configuaration *******************************
*******************************************************************************/
/**
  * Enumeration represents type of class.
  */
typedef enum rcc_class_type_t {
    RCC_CLASS_INVALID = 0,	/**< Invalid value */
    RCC_CLASS_STANDARD,		/**< Standard class */
    RCC_CLASS_KNOWN,		/**< Class encoding is known and no autodetection should be performed */
    RCC_CLASS_FS		/**< Class strings are representing file names */
} rcc_class_type;

/**
  * Provides information about default encoding for specific language 
  */
struct rcc_class_default_charset_t {
    const char *lang;		/**< Language */
    const char *charset;	/**< Default encoding for #lang */
};
typedef const struct rcc_class_default_charset_t rcc_class_default_charset;

/** Forbid change class value using Library API */
#define RCC_CLASS_FLAG_CONST			0x01
/** Forbid saving and loading of class value */
#define RCC_CLASS_FLAG_SKIP_SAVELOAD		0x02	

/** Encoding class description. Encoding classes are main concept of LibRCC library.
  * The strings are recoded between different classes (for example in RusXMMS2 project
  * ID3 titles are recoded between ID3 and Output classes). The current encoding of
  * each class can be set using configuration file or API call, otherwise it will
  * be detected automatically using current locale or default encoding.
  *
  * If the #defvalue is not NULL, it provides information about detection of
  * the default encoding. The are following possibilities for that value:
  *	- Detect default encoding using specified locale variable (LC_CTYPE for example).
  * .	- The current encoding of the another class will be used instead of default encodings. Short name of considered class should be specified.
  *	- Just use specified multibyte encoding for all languages.
  * In case of detection failure using all these methods, the #defcharset will 
  * be examined if default encoding for current language is available. If not,
  * the first encoding in the list will be used as current.
  * 
  * Additionaly it is possible to set special flags to prevent user from 
  * modifying class value. It is possible to protect class from changing
  * ether using API, or configuration files.
  *
  * @see rcc_class_default_charset_t
  * @see RCC_CLASS_FLAG_CONST
  * @see RCC_CLASS_FLAG_SKIP_SAVELOAD
  *
  * The class type provides information for recoding functions about 
  * automatic detection of the class chrset. The encodings of the
  * #RCC_CLASS_STANDARD classes will be detected using autoengine (if available
  * for current language. The #RCC_CLASS_FS classes are associated with files
  * and encoding will be guessed using find_file.
  *
  * @see rcc_class_type_t
  * 
  */
struct rcc_class_t {
    const char *name;	/**< Short class name */
    const rcc_class_type class_type; /**< specifies type of class (Standard, File System, Known) */
    const char *defvalue; /**< locale variable name or parrent name or multibyte encoding name */
    rcc_class_default_charset *defcharset; /**< default class encodings. Should be specified on per-language basys */
    const char *fullname; /**< Full name of the class */
    const unsigned long flags; /**< Class flags. (CONST, SKIP_SAVELOAD) */
};
typedef const struct rcc_class_t rcc_class;
typedef rcc_class_ptr rcc_class_list[RCC_MAX_CLASSES+1];

/**
  * Register additional class
  * @param ctx is working context ( or default one if NULL supplied )
  * @param cl is pointer on the class description (shouldn't be freed before library deinitialization).
  * @return registered class id  or -1 in case of a error.
  */
rcc_class_id rccRegisterClass(rcc_context ctx, rcc_class *cl);
/**
  * Determines 'class type' of supplied class.
  * @param ctx is working context ( or default one if NULL supplied )
  * @param class_id is class id
  * @return class type  or -1 in case of a error.
  */
rcc_class_type rccGetClassType(rcc_context ctx, rcc_class_id class_id);

/*******************************************************************************
************************ Altering Configuaration *******************************
*******************************************************************************/
typedef int rcc_option_value;

/**
  * Use BerkeleyDB recodings cache for encoding detection
  */
#define RCC_OPTION_LEARNING_FLAG_USE 1
/**
  * Cache recodings in BerkeleyDB recoding cache for future use
  */
#define RCC_OPTION_LEARNING_FLAG_LEARN 2

/**
  * Switch translation off.
  */
#define RCC_OPTION_TRANSLATE_OFF 0
/**
  * Translate data to english language (Current language don't matter).
  */
#define RCC_OPTION_TRANSLATE_TO_ENGLISH 1
/**
  * Skip translation of the english text.
  */
#define RCC_OPTION_TRANSLATE_SKIP_ENGLISH 2
/**
  * Translate whole data to the current language.
  */
#define RCC_OPTION_TRANSLATE_FULL 3

/**
  * List of options available
  */
typedef enum rcc_option_t {
    RCC_OPTION_LEARNING_MODE = 0,	/**< Recoding Caching mode (OFF/ON/RELEARN/LEARN) */
    RCC_OPTION_AUTODETECT_FS_TITLES,	/**< Detect titles of #RCC_CLASS_FS classes */
    RCC_OPTION_AUTODETECT_FS_NAMES,	/**< Try to find encoding of #RCC_CLASS_FS by accessing fs */
    RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, /**< Use only configured languages or languages with auto-engines */
    RCC_OPTION_AUTOENGINE_SET_CURRENT,	/**< If enabled autodetection engine will set current charset */
    RCC_OPTION_AUTODETECT_LANGUAGE,	/**< Enables language detection */
    RCC_OPTION_TRANSLATE,		/**< Translate #rcc_string if it's language differs from current one */
    RCC_MAX_OPTIONS
} rcc_option;

/**
  * List of option types
  */
typedef enum rcc_option_type_t {
    RCC_OPTION_TYPE_INVISIBLE = 0,	/**< Invisible option. Wouldn't be represented in UI menu */
    RCC_OPTION_TYPE_STANDARD,		/**< Standard option. */
    RCC_OPTION_TYPE_MAX
} rcc_option_type;

/**
  * Description of option values range type
  */
typedef enum rcc_option_range_type_t {
    RCC_OPTION_RANGE_TYPE_BOOLEAN = 0,	/**< Boolean option */
    RCC_OPTION_RANGE_TYPE_RANGE,	/**< Range of integer values */
    RCC_OPTION_RANGE_TYPE_FLAGS,	/**< Set of boolean flags */
    RCC_OPTION_RANGE_TYPE_MENU,		/**< Enumeration */
    RCC_OPTION_RANGE_TYPE_MAX
} rcc_option_range_type;

/**
  * Descriptionm of value range 
  */
typedef struct rcc_option_range_t {
    rcc_option_range_type type;		/**< Value range type */
    rcc_option_value min;		/**< Minimal acceptable option value */
    rcc_option_value max;		/**< Maximal acceptable option value */
    rcc_option_value step;		/**< Preccision step */
}rcc_option_range;

/* lng.c */
/**
  * Determines name of the supplied language.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param language_id is 'language id' of desired language. For default language the 'default' value will be returned.
  * @return language name  or NULL in case of a error.
  */
const char *rccGetLanguageName(rcc_context ctx, rcc_language_id language_id);
/**
  * Finds language id by the supplied name.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param name is language name
  * @return language id [0-n] or -1 if not found.
  */
rcc_language_id rccGetLanguageByName(rcc_context ctx, const char *name);
/**
  * This function resolves default languages. If positive language id is supplied
  * it will be returned back unchanged. The default language (0 is supplied as
  * language id) will be resolved to some particular language.
  * The following procedure will be used:
  *	- 1. Detect Language by locale
  *	- 2. Check if language intialized if RCC_OPTION_CONFIGURED_LANGUAGES_ONLY is set
  *	- 3. If one of the previous steps is failed, select first available language (id=1). Usually it should be 'LibRCC off'.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param language_id is language id
  * @return resolved language id [1-n] or -1 in case of error.
  */
rcc_language_id rccGetRealLanguage(rcc_context ctx, rcc_language_id language_id);
/**
  * Return considered language name, resolving default language if necessary.
  * @see rccGetRealLanguage
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param language_id is language id
  * @return resolved language name or NULL in case of error.
  */
const char *rccGetRealLanguageName(rcc_context ctx, rcc_language_id language_id);
/**
  * Return selected language id.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @return selected language id [0-n] or -1 in case of error
  */
rcc_language_id rccGetSelectedLanguage(rcc_context ctx);
/**
  * Return selected language name.
  * @see rccGetSelectedLanguage
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @return selected language name or NULL in case of error.
  */
const char *rccGetSelectedLanguageName(rcc_context ctx);
/**
  * Return current language id, resolving default language to particular one if necessary.
  * See more details how default language is resolved: @see rccGetRealLanguage
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @return current language id [1-n] or -1 in case of error
  */
rcc_language_id rccGetCurrentLanguage(rcc_context ctx);
/**
  * Return current language name.
  # @see rccGetCurrentLanguage
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @return current language id [1-n] or -1 in case of error
  */
const char *rccGetCurrentLanguageName(rcc_context ctx);


/**
  * Set current language.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param language_id is new language id [0-n]. Set to default state is Ok.
  * @return non-zero value in case of error
  */
int rccSetLanguage(rcc_context ctx, rcc_language_id language_id);
/**
  * Set current language by name.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param name is the short name of new language.
  * @return non-zero value in case of error
  */
int rccSetLanguageByName(rcc_context ctx, const char *name);

/* opt.c */
/**
  * Return option value.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param option is option
  * @return current option value or -1 in case of error
  */
rcc_option_value rccGetOption(rcc_context ctx, rcc_option option);
/**
  * Tests if option have unchanged default value.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param option is option
  * @return current option value or -1 in case of error
  */
int rccOptionIsDefault(rcc_context ctx, rcc_option option);
/**
  * Sets option to its default value.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param option is option
  * @return non-zero value in case of error
  */
int rccOptionSetDefault(rcc_context ctx, rcc_option option);
/**
  * Set option value.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param option is option
  * @param value is option value
  * @return non-zero value in case of erros
  */
int rccSetOption(rcc_context ctx, rcc_option option, rcc_option_value value);
/**
  * Get current option type.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param option is option
  * @return current option type or -1 in case of error
  */
rcc_option_type rccOptionGetType(rcc_context ctx, rcc_option option);
/**
  * Return range description for specified option
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param option is option
  * @return option range or -1 in case of error
  */
rcc_option_range *rccOptionGetRange(rcc_context ctx, rcc_option option);

/**
  * Get short name of supplied option.
  *
  * @param option is option
  * @return option range or NULL in case of error
  */
const char *rccGetOptionName(rcc_option option);

/**
  * Get short name of supplied option value.
  *
  * @param option is option 
  * @param value is value of #option
  * @return option value name or NULL in case of error
  */
const char *rccGetOptionValueName(rcc_option option, rcc_option_value value);
/**
  * Get option by short name.
  *
  * @param name is option name
  * @return option or -1 in case of error
  */
rcc_option rccGetOptionByName(const char *name);
/**
  * Get option value by short name.
  *
  * @param option is option 
  * @param name is value name
  * @return option value or -1 in case of error
  */
rcc_option_value rccGetOptionValueByName(rcc_option option, const char *name);


/* lngconfig.c */
/**
  * Check if configuration is initialized for supplied language. 
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param language_id is concerned language id
  * @return configuration context if:
  * 	- language_id is particular language, not default one
  *	- language already intialized 
  *	- language is not dummy (Disable LibRCC) language
  * otherwise NULL is returned
  */
rcc_language_config rccCheckConfig(rcc_context ctx, rcc_language_id language_id);
/**
  * Initializes language configuration if not yet configured and returns pointer on
  * that configuration. If default language is supplied (language_id = 0), the 
  * language id will be resolved to particular language and config of that language
  * will be returned.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param language_id is concerned language id
  * @return configuration context. The NULL is returned in the case of errors or 
  * dummy (Dissable LibRCC) language is selected.
  */
rcc_language_config rccGetConfig(rcc_context ctx, rcc_language_id language_id);
/**
  * Initializes language configuration if not yet configured and returns pointer on
  * that configuration.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param name is concerned language name
  * @return configuration context or NULL in case of error
  */
rcc_language_config rccGetConfigByName(rcc_context ctx, const char *name);
/**
  * Returns pointer on the current language configuration (Initializes it as well
  * if required)
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @return configuration context or NULL in case of error
  */
rcc_language_config rccGetCurrentConfig(rcc_context ctx);

/**
  * Return supplied engine name
  *
  * @param config is language configuration
  * @param engine_id is desired engine 
  * @return selected engine name or NULL in case of error.
  */
const char *rccConfigGetEngineName(rcc_language_config config, rcc_engine_id engine_id);
/**
  * Return supplied encoding name
  *
  * @param config is language configuration
  * @param charset_id is desired charset 
  * @return selected encoding name or NULL in case of error.
  */
const char *rccConfigGetCharsetName(rcc_language_config config, rcc_charset_id charset_id);
/**
  * Function finds engine id by the supplied name.
  *
  * @param config is language configuration
  * @param name is engine name
  * @return engine id [0-n] or -1 if not found 
  */
rcc_engine_id rccConfigGetEngineByName(rcc_language_config config, const char *name);
/**
  * Function finds encoding id by the supplied name.
  *
  * @param config is language configuration
  * @param name is encoding name
  * @return encoding id [0-n] or -1 if not found.
  */
rcc_charset_id rccConfigGetCharsetByName(rcc_language_config config, const char *name);

/**
  * Return selected engin id.
  *
  * @param config is language configuration
  * @return selected engine id [-1-n]
  * 	- -1 engine is not configured and first available will be used
  *	-  0 engines are dissabled
  *	- >0 paticular engine id
  */
rcc_engine_id rccConfigGetSelectedEngine(rcc_language_config config);
/**
  * Return selected engine name.
  * @see rccConfigGetSelectedEngine
  *
  * @param config is language configuration
  * @return selected engine name ('default' will be returned if engine not configured) or NULL in case of error.
  */
const char *rccConfigGetSelectedEngineName(rcc_language_config config);
/**
  * Return current engine_id. The default value will be resolved to paticular engine id. Normally,
  * the id of the first available engine will be returned. If no engines registered for supplied
  * language the 0 will be returned, indicating id of dummy(dissabled) engine.
  *
  * @param config is language configuration
  * @return selected engine id [0-n] or -1 in case of error
  *	-  0 engines are dissabled
  *	- >0 paticular engine id
  */
rcc_engine_id rccConfigGetCurrentEngine(rcc_language_config config);
/**
  * Return current engine name.
  * @see rccConfigGetCurrentEngine
  *
  * @param config is language configuration
  * @return current engine name or NULL in case of error.
  */
const char *rccConfigGetCurrentEngineName(rcc_language_config config);

/**
  * Return selected encoding id.
  *
  * @param config is language configuration
  * @param class_id is encoding class
  * @return selected encoding id [0-n] or -1 in case of error
  *	-  0 default encoding
  *	- >0 paticular encoding id
  */
rcc_charset_id rccConfigGetSelectedCharset(rcc_language_config config, rcc_class_id class_id);
/**
  * Return selected encoding name.
  * @see rccConfigGetSelectedCharset
  *
  * @param config is language configuration
  * @param class_id is charset encodings
  * @return selected encodings name ('default' will be returned if engine not configured) or NULL in case of error.
  */
const char *rccConfigGetSelectedCharsetName(rcc_language_config config, rcc_class_id class_id);
/**
  * Return current encoding_id. The default value will be resolved to paticular encoding id. 
  * The following procedure is used to detect default encoding:
  *	- If the parrent class is defined in #defcharset, - return current encoding of parrent class.
  *	- If the locale variable is defined in #defcharset and config language coincide with locale language, use locale encoding.
  *	- If the default value for config language is defined in #defvalue return that default value.
  *	- Return language with id 0. Normally this should be dummy language which indicates that RCC library is not used.
  *
  * @param config is language configuration
  * @param class_id is encoding class
  * @return selected charset id [1-n] or -1 in case of error
  */
rcc_charset_id rccConfigGetCurrentCharset(rcc_language_config config, rcc_class_id class_id);
/**
  * Return current encoding name.
  * @see rccConfigGetCurrentCharset
  *
  * @param config is language configuration
  * @param class_id is encoding class
  * @return current charset name or NULL in case of error.
  */
const char *rccConfigGetCurrentCharsetName(rcc_language_config config, rcc_class_id class_id);

/**
  * Set current engine.
  *
  * @param config is language configuration
  * @param engine_id is new language id [-1-n]. If -1 supplied the engine will go in non-configured state.
  * @return non-zero value in case of error
  */
int rccConfigSetEngine(rcc_language_config config, rcc_engine_id engine_id);
/**
  * Set current encoding.
  *
  * @param config is language configuration
  * @param class_id is encoding class
  * @param charset_id is new charset id [0-n]. The 0 will switch charset to encoding state.
  * @return non-zero value in case of error
  */
int rccConfigSetCharset(rcc_language_config config, rcc_class_id class_id, rcc_charset_id charset_id);
/**
  * Set current engine by name.
  *
  * @param config is language configuration
  * @param name is the short name of new engine ("default" is okey).
  * @return non-zero value in case of error
  */
int rccConfigSetEngineByName(rcc_language_config config, const char *name);
/**
  * Set current encoding by name.
  *
  * @param config is language configuration
  * @param class_id is encoding class
  * @param name is the short name of new encoding ("default" is okey).
  * @return non-zero value in case of error
  */
int rccConfigSetCharsetByName(rcc_language_config config, rcc_class_id class_id, const char *name);

/**
  * Function will return encoding id of charset specified by locale configuration.
  *
  * @param config is language configuration
  * @param locale_variable is locale variable (Default(NULL) is LC_CTYPE)
  * @return encoding id
  */
rcc_charset_id rccConfigGetLocaleCharset(rcc_language_config config, const char *locale_variable);


/* curconfig.c */
const char *rccGetEngineName(rcc_context ctx, rcc_engine_id engine_id);
const char *rccGetCharsetName(rcc_context ctx, rcc_charset_id charset_id);

rcc_engine_id rccGetEngineByName(rcc_context ctx, const char *name);
rcc_charset_id rccGetCharsetByName(rcc_context ctx, const char *name);

rcc_engine_id rccGetSelectedEngine(rcc_context ctx);
const char *rccGetSelectedEngineName(rcc_context ctx);
rcc_engine_id rccGetCurrentEngine(rcc_context ctx);
const char *rccGetCurrentEngineName(rcc_context ctx);
rcc_charset_id rccGetSelectedCharset(rcc_context ctx, rcc_class_id class_id);
const char *rccGetSelectedCharsetName(rcc_context ctx, rcc_class_id class_id);
rcc_charset_id rccGetCurrentCharset(rcc_context ctx, rcc_class_id class_id);
const char *rccGetCurrentCharsetName(rcc_context ctx, rcc_class_id class_id);

int rccSetEngine(rcc_context ctx, rcc_engine_id engine_id);
int rccSetCharset(rcc_context ctx, rcc_class_id class_id, rcc_charset_id charset_id);
int rccSetEngineByName(rcc_context ctx, const char *name);
int rccSetCharsetByName(rcc_context ctx, rcc_class_id class_id, const char *name);

rcc_charset_id rccGetLocaleCharset(rcc_context ctx, const char *locale_variable);

/*******************************************************************************
************************ Language Configuaration *******************************
*******************************************************************************/
/* rcclist.c */
rcc_language_ptr *rccGetLanguageList(rcc_context ctx);
rcc_charset *rccGetCharsetList(rcc_context ctx, rcc_language_id language_id);
rcc_engine_ptr *rccGetEngineList(rcc_context ctx, rcc_language_id language_id);
rcc_charset *rccGetCurrentCharsetList(rcc_context ctx);
rcc_engine_ptr *rccGetCurrentEngineList(rcc_context ctx);
rcc_charset *rccGetCurrentAutoCharsetList(rcc_context ctx);
rcc_class_ptr *rccGetClassList(rcc_context ctx);

/*******************************************************************************
************************ RCC_STRING Manipulations ******************************
*******************************************************************************/
/* string.c */
/**
  * Intermediate string format. RCC_string can be manipulated as standard NULL terminated string. 
  * However it contains small header with information about string language. All strings are
  * encoded using UTF-8 encoding.
  */
typedef char *rcc_string;
/**
  * Intermediate string format. RCC_string can be manipulated as standard NULL terminated string. 
  * However it contains small header with information about string language. All strings are
  * encoded using UTF-8 encoding.
  */
typedef const char *rcc_const_string;

/**
  * Check string header and verify if it is really correct #rcc_string.
  *
  * @param str is verifying string
  */
size_t rccStringCheck(const char *str);
/**
  * Check string header and verify if it is really correct #rcc_string.
  *
  * @param str is verifying string
  * @param len is preciese size of str.
  * @return size of string in bytes or -1 if check failed
  */
size_t rccStringSizedCheck(const char *str, size_t len);

/**
  * Extract language from #rcc_string.
  *
  * @param str is #rcc_string
  * @return size of string in bytes or -1 if check failed
  */
rcc_language_id rccStringGetLanguage(rcc_const_string str);
/**
  * Returns pointer on UTF-8 string kept inside of #rcc_string.
  *
  * @param str is #rcc_string
  * @return pointer on constant string or NULL in the case of error
  */
const char *rccStringGetString(rcc_const_string str);
/**
  * Extract UTF-8 string from #rcc_string.
  *
  * @param str is #rcc_string
  * @return pointer on string or NULL in the case of error. The string should be freed by the caller.
  */
char *rccStringExtractString(rcc_const_string str);

/**
  * If str is #rcc_string function will return pointer on UTF-8 string kept inside, otherwise
  * pointer on the passed string is returned.
  *
  * @param str is null-terminated string
  * @return pointer on constant string
  */
const char *rccGetString(const char *str);
/**
  * If str is #rcc_string function will return pointer on UTF-8 string kept inside, otherwise
  * pointer on the passed string is returned.
  *
  * @param str is string (perhaps not zero terminated)
  * @param len is exact length of string or 0 (in this case length will be computed using 'strlen'
  * @return pointer on constant string
  */
const char *rccSizedGetString(const char *str, size_t len);

int rccStringCmp(const char *str1, const char *str2);
int rccStringNCmp(const char *str1, const char *str2, size_t n);
int rccStringCaseCmp(const char *str1, const char *str2);
int rccStringNCaseCmp(const char *str1, const char *str2, size_t n);

/*******************************************************************************
******************************** Recoding **************************************
*******************************************************************************/
/**
  * recoding context
  */
typedef struct rcc_iconv_t *rcc_iconv;

/* rcciconv.c */
/**
  * Open recoding context.
  *
  * @param from is source encoding
  * @param to is destination encoding
  * @result 
  *	- NULL if no recoding is required
  *	- Pointer on initialized context if successful
  */
rcc_iconv rccIConvOpen(const char *from, const char *to);
/**
  * Close recoding context.
  *
  * @param icnv is recoding context */
void rccIConvClose(rcc_iconv icnv);
/** 
  * Recodes chunk of data.
  *
  * @param icnv is recoding context
  * @param buf is data for recoding
  * @param len is size of the data
  * @param rlen is size of recoded data
  * @return recoded string or NULL in the case of error
  */
char *rccIConv(rcc_iconv icnv, const char *buf, size_t len, size_t *rlen);

/**
  * translating context
  */
typedef struct rcc_translate_t *rcc_translate;

/* rcctranslate.c */
/**
  * Open translating context.
  *
  * @param from is source language
  * @param to is destination language
  * @return
  *	- NULL if translation is not required or possible
  *	- Pointer on initialized context if successful
  */
rcc_translate rccTranslateOpen(const char *from, const char *to);
/**
  * Close translating context.
  *
  * @param translate is translating context
  */
void rccTranslateClose(rcc_translate translate);

/*
 * Set translation timeout
 *
 * @param translate is translating context 
 * @param us is timeout in microseconds (0 - no timeout)
 * @return non-zero value is returned in the case of errror
 */
int rccTranslateSetTimeout(rcc_translate translate, unsigned long us);

/** 
  * Translate string.
  *
  * @param translate is translating context
  * @param buf is UTF-8 encoded string for translating
  * @return recoded string or NULL in the case of error
  */
char *rccTranslate(rcc_translate translate, const char *buf);

/* recode.c */

/**
  * Tries to detect language of string
  * @param ctx is working context ( or default one if NULL supplied )
  * @param class_id is encoding class
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @result is language_id or -1 if autodetection is failed
  */
rcc_language_id rccDetectLanguage(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len);
/**
  * Tries to detect charset of string
  * @param ctx is working context ( or default one if NULL supplied )
  * @param class_id is encoding class
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @result is auto_charset_id or -1 if autodetection is failed
  */
int rccDetectCharset(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len);

/**
  * Recode string from specified encoding class to #rcc_string. Encoding detection engines and
  * recoding cache are used (if possible) to detect original 'buf' encoding. Otherwise the 
  * preconfigured encoding of class is assumed.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param class_id is encoding class
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
rcc_string rccSizedFrom(rcc_context ctx, rcc_class_id class_id, const char *buf, size_t len);
/**
  * Recode string from #rcc_string to specified encoding class. If encoding class is of 
  * 'File System' type, the autoprobing for file names can be performed. In the other cases
  * the rcc_string will be recoded in preconfigured class encoding.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param class_id is encoding class
  * @param buf is original zero terminated string
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccSizedTo(rcc_context ctx, rcc_class_id class_id, rcc_const_string buf, size_t *rlen);
/**
  * Recode string between different encoding classes. The conversion is relays on rccSizedFrom
  * and rccSizedTo functions.
  * @see rccSizedFrom
  * @see rccSizedTo
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param from is source encoding class
  * @param to is destination encoding class
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccSizedRecode(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *buf, size_t len, size_t *rlen);
/**
  * Recode file name between different encoding classes. Normally, should be used to convert 
  * string from the file list to real file system names. The autoprobing of names is enabled 
  * depending on the output encoding class configuration and current options.
  * @see rcc_class_t
  * @see rcc_class_type_t
  * @see rcc_option_t
  
  * The conversion is partly relays on rccSizedFrom.
  * @see rccSizedFrom
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param from is source encoding class
  * @param to is destination encoding class
  * @param fspath is path to the filesystem where file are located. 
  * @param path is file path
  * @param filename is file name
  * @result is recoded file name or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccFS(rcc_context ctx, rcc_class_id from, rcc_class_id to, const char *fspath, const char *path, const char *filename);

/**
  * Recode string from specified encoding to #rcc_string. 
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param charset is source encoding
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
rcc_string rccSizedFromCharset(rcc_context ctx, const char *charset, const char *buf, size_t len);
/**
  * Recode string from #rcc_string to specified encoding. 
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param charset is destination encoding
  * @param buf is original zero terminated string
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccSizedToCharset(rcc_context ctx, const char *charset, rcc_const_string buf, size_t *rlen);
/**
  * Recode string between specified encoding class and encoding. 
  *
  * The conversion is partly relays on rccSizedFrom.
  * @see rccSizedFrom
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param class_id is source encoding class
  * @param charset is destination encoding
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccSizedRecodeToCharset(rcc_context ctx, rcc_class_id class_id, const char *charset, rcc_const_string buf, size_t len, size_t *rlen);
/**
  * Recode string between specified encoding and encoding class. 
  *
  * The conversion is partly relays on rccSizedTo
  * @see rccSizedTo
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param class_id is destination encoding class
  * @param charset is source encoding
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
rcc_string rccSizedRecodeFromCharset(rcc_context ctx, rcc_class_id class_id, const char *charset, const char *buf, size_t len, size_t *rlen);
/**
  * Recode string between specified encodings. 
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param from is source encoding
  * @param to is destination encoding
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccSizedRecodeCharsets(rcc_context ctx, const char *from, const char *to, const char *buf, size_t len, size_t *rlen);


/**
  * Tries to detect charset of string
  * @param config is language configuration
  * @param class_id is encoding class
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @result is auto_charset_id or -1 if autodetection is failed
  */
rcc_autocharset_id rccConfigDetectCharset(rcc_language_config config, rcc_class_id class_id, const char *buf, size_t len);

/**
  * Recode string from specified encoding class to #rcc_string. Encoding detection engines and
  * recoding cache are used (if possible) to detect original 'buf' encoding. Otherwise the 
  * preconfigured encoding of class is assumed.
  *
  * @param config is language configuration
  * @param class_id is encoding class
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
rcc_string rccConfigSizedFrom(rcc_language_config config, rcc_class_id class_id, const char *buf, size_t len);
/**
  * Recode string from #rcc_string to specified encoding class. If encoding class is of 
  * 'File System' type, the autoprobing for file names can be performed. In the other cases
  * the rcc_string will be recoded in preconfigured class encoding.
  *
  * @param config is language configuration
  * @param class_id is encoding class
  * @param buf is original zero terminated string
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccConfigSizedTo(rcc_language_config config, rcc_class_id class_id, rcc_const_string buf, size_t *rlen);
/**
  * Recode string between different encoding classes. The conversion is relays on rccConfigSizedFrom
  * and rccConfigSizedTo functions.
  * @see rccConfigSizedFrom
  * @see rccConfigSizedTo
  *
  * @param config is language configuration
  * @param from is source encoding class
  * @param to is destination encoding class
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccConfigSizedRecode(rcc_language_config config, rcc_class_id from, rcc_class_id to, const char *buf, size_t len, size_t *rlen);
/**
  * Recode string from specified encoding to #rcc_string. 
  *
  * @param config is language configuration
  * @param charset is source encoding
  * @param buf is original string (perhaps not zero terminated)
  * @param len is exact size of string or 0. In the last case the size is determined using 'strlen' function.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
rcc_string rccConfigSizedRecodeFromCharset(rcc_language_config config, rcc_class_id class_id, const char *charset, const char *buf, size_t len, size_t *rlen);
/**
  * Recode string from #rcc_string to specified encoding. 
  *
  * @param config is language configuration
  * @param charset is destination encoding
  * @param buf is original zero terminated string
  * @param rlen in rlen the size of recoded string will be returned.
  * @result is recoded string or NULL if recoding is not required or failed. It is up to the caller to free memory.
  */
char *rccConfigSizedRecodeToCharset(rcc_language_config config, rcc_class_id class_id, const char *charset, rcc_const_string buf, size_t len, size_t *rlen);


#define rccFrom(ctx, class_id, buf) rccSizedFrom(ctx, class_id, buf, 0)
#define rccTo(ctx, class_id, buf) rccSizedTo(ctx, class_id, buf, NULL)
#define rccRecode(ctx, from, to, buf) rccSizedRecode(ctx, from, to, buf, 0, NULL)

#define rccFromCharset(ctx, charset, buf) rccSizedFromCharset(ctx, charset, buf, 0)
#define rccToCharset(ctx, charset, buf) rccSizedToCharset(ctx, charset, buf, NULL)
#define rccRecodeToCharset(ctx, class_id, charset, buf) rccSizedRecodeToCharset(ctx, class_id, charset, buf, 0, NULL)
#define rccRecodeFromCharset(ctx, class_id, charset, buf) rccSizedRecodeFromCharset(ctx, class_id, charset, buf, 0, NULL)
#define rccRecodeCharsets(ctx, from, to, buf) rccSizedRecodeCharsets(ctx, from, to, buf, 0, NULL)

#define rccConfigFrom(ctx, class_id, buf) rccConfigSizedFrom(ctx, class_id, buf, 0)
#define rccConfigTo(ctx, class_id, buf) rccConfigSizedTo(ctx, class_id, buf, NULL)
#define rccConfigRecode(ctx, from, to, buf) rccConfigSizedRecode(ctx, from, to, buf, 0, NULL)
#define rccConfigRecodeToCharset(ctx, class_id, charset, buf) rccConfigSizedRecodeToCharset(ctx, class_id, charset, buf, 0, NULL)
#define rccConfigRecodeFromCharset(ctx, class_id, charset, buf) rccConfigSizedRecodeFromCharset(ctx, class_id, charset, buf, 0, NULL)

/*******************************************************************************
******************************** Options ***************************************
*******************************************************************************/

/* xml.c */
typedef void *rcc_config;
rcc_config rccGetConfiguration();

/**
  * Save Configuration.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param name is configuration file name ( can be shared between multiple applications! )
  * @return non-zero in the case of errors.
  */
int rccSave(rcc_context ctx, const char *name);
/**
  * Load Configuration.
  *
  * @param ctx is working context ( or default one if NULL supplied )
  * @param name is configuration file name ( can be shared between multiple applications! )
  * @return non-zero in the case of errors.
  */
int rccLoad(rcc_context ctx, const char *name);

/*******************************************************************************
**************************** Engine Plugins ************************************
*******************************************************************************/

typedef rcc_engine *(*rcc_plugin_engine_info_function)(const char *lang);

rcc_engine_internal rccEngineGetInternal(rcc_engine_context ctx);
rcc_language *rccEngineGetLanguage(rcc_engine_context ctx);
rcc_context rccEngineGetRccContext(rcc_engine_context ctx);

/*******************************************************************************
**************************** Configuration *************************************
*******************************************************************************/

/**
  * The Berkley DB support is compiled in
  */
#define RCC_CC_FLAG_HAVE_BERKLEY_DB 			0x01
/**
  * The dynamic engine plugins support is compiled in
  */
#define RCC_CC_FLAG_HAVE_DYNAMIC_ENGINES		0x02
/**
  * Enca engine is compiled in 
  */
#define RCC_CC_FLAG_HAVE_ENCA				0x04
/**
  * LibRCD engine is compiled in
  */
#define RCC_CC_FLAG_HAVE_RCD				0x08
/**
  * Libtranslate translation engine compiled in
  */
#define RCC_CC_FLAG_HAVE_LIBTRANSLATE			0x10

/**
  * The library build environment is represented by this structure
  */ 
struct rcc_compiled_configuration_t {
    unsigned long flags;	/**< compilation flags */
};
typedef struct rcc_compiled_configuration_t rcc_compiled_configuration_s;
typedef const struct rcc_compiled_configuration_t *rcc_compiled_configuration;

/**
  * Get information about library compilation environment
  */
rcc_compiled_configuration rccGetCompiledConfiguration();

int rccLocaleGetClassByName(const char *locale);
int rccLocaleGetLanguage(char *result, const char *lv, unsigned int n);
int rccLocaleGetCharset(char *result, const char *lv, unsigned int n);

#ifdef __cplusplus
}
#endif

#endif /* _LIBRCC_H */
