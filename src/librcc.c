#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "../config.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif /* HAVE_PWD_H */

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif /* HAVE_SYS_FILE_H */


#include "internal.h"
#include "rccconfig.h"
#include "rccenca.h"
#include "rcclist.h"
#include "plugin.h"
#include "engine.h"
#include "rccxml.h"
#include "rccexternal.h"
#include "rcctranslate.h"

static int initialized = 0;
char *rcc_home_dir = NULL;
rcc_context rcc_default_ctx = NULL;
static rcc_compiled_configuration_s compiled_configuration;

rcc_compiled_configuration rccGetCompiledConfiguration() {
    compiled_configuration.flags = 0;
#ifdef HAVE_RCD
    compiled_configuration.flags|=RCC_CC_FLAG_HAVE_RCD;
#endif /* HAVE_RCD */
#ifdef HAVE_ENCA
    compiled_configuration.flags|=RCC_CC_FLAG_HAVE_ENCA;
#endif /* HAVE_ENCA */
#ifdef HAVE_DLOPEN
    compiled_configuration.flags|=RCC_CC_FLAG_HAVE_DYNAMIC_ENGINES;
#endif /* HAVE_DLOPEN */
#ifdef HAVE_DB_H
    compiled_configuration.flags|=RCC_CC_FLAG_HAVE_BERKLEY_DB;
#endif /* HAVE_DB_H */
#ifdef HAVE_LIBTRANSLATE
    compiled_configuration.flags|=RCC_CC_FLAG_HAVE_LIBTRANSLATE;
#endif /* HAVE_LIBTRANSLATE */

    return &compiled_configuration;
}

int rccInit() {
    int err;
    char *tmp;
    unsigned long i, rpos;

#ifdef HAVE_PWD_H
    struct passwd *pw;
#endif /* HAVE_PWD_H */
    
    if (initialized) return 0;
    
    tmp = getenv ("HOME");
    if (tmp) rcc_home_dir = strdup (tmp);
#ifdef HAVE_PWD_H
    else {
	setpwent ();
	pw = getpwuid(getuid ());
	endpwent ();
	if ((pw)&&(pw->pw_dir)) rcc_home_dir = strdup (pw->pw_dir);
    }
#endif /* HAVE_PWD_H */
    if (!rcc_home_dir) rcc_home_dir = strdup("/");

    memcpy(rcc_default_languages, rcc_default_languages_embeded, (RCC_MAX_LANGUAGES + 1)*sizeof(rcc_language));
    memcpy(rcc_default_aliases, rcc_default_aliases_embeded, (RCC_MAX_ALIASES + 1)*sizeof(rcc_language_alias));
    memcpy(rcc_default_relations, rcc_default_relations_embeded, (RCC_MAX_RELATIONS + 1)*sizeof(rcc_language_relation));
    memcpy(rcc_option_descriptions, rcc_option_descriptions_embeded, (RCC_MAX_OPTIONS + 1)*sizeof(rcc_option_description));

#ifdef HAVE_LIBTRANSLATE
    rccExternalInit();
#endif /* HAVE_LIBTRANSLATE */    

    for (rpos=0;rcc_default_relations[rpos].lang;rpos++);
    for (i=0;rcc_default_languages[i].sn;i++) {
	if (!strcasecmp(rcc_default_languages[i].sn, rcc_default_language_sn)) continue;
	if (!strcasecmp(rcc_default_languages[i].sn, rcc_disabled_language_sn)) continue;
	if (!strcasecmp(rcc_default_languages[i].sn, rcc_english_language_sn)) continue;

	rcc_default_relations[rpos].lang = rcc_default_languages[i].sn;
	rcc_default_relations[rpos++].parrent = rcc_english_language_sn;
    }
    rcc_default_relations[rpos].lang = NULL;
    rcc_default_relations[rpos].parrent = NULL;

    err = rccPluginInit();
    if (!err) err = rccTranslateInit();
    if (!err) err = rccXmlInit(1);
    if (!err) err = rccEngineInit();

    if (err) {
	rccFree();
	return err;
    }

    initialized = 1;

    return 0;
}

void rccFree() {
    if (rcc_default_ctx) {
	rccFreeContext(rcc_default_ctx);
	rcc_default_ctx = NULL;
    }
    
    rccEngineFree();
    rccXmlFree();
    rccTranslateFree();
    rccPluginFree();

    rccExternalFree();

    if (rcc_home_dir) {
	free(rcc_home_dir);
	rcc_home_dir = NULL;
    }

    initialized = 0;
}

static int lockfd = -1;

int rccLock() {
# ifdef HAVE_SYS_FILE_H
    int err, i;
    int size;
    char *stmp;
    struct timespec wait = { 0, 10000000 };
    
    if (lockfd>=0) return -1;    
    
    size = strlen(rcc_home_dir) + 32;
    stmp = (char*)malloc(size*sizeof(char));
    if (!stmp) return -1;

    sprintf(stmp,"%s/.rcc/", rcc_home_dir);
    mkdir(stmp, 00755);
    
    sprintf(stmp,"%s/.rcc/locks/", rcc_home_dir);
    mkdir(stmp, 00700);

    sprintf(stmp,"%s/.rcc/locks/rcc.lock", rcc_home_dir);

    lockfd = open(stmp, O_RDWR|O_CREAT, 0644);
    if (lockfd >= 0) {
	for (err = -1, i = 0; i < (LIBRCC_LOCK_WAIT/10); i++) {
	    err = flock(lockfd, LOCK_EX|LOCK_NB);
	    if ((err)&&(errno == EWOULDBLOCK)) nanosleep(&wait, NULL);
	    else break;
	}

	if (err) {
	    close(lockfd);

		// Removing invalid lock
	    if (i == (LIBRCC_LOCK_WAIT/10)) {
		remove(stmp);

		lockfd = open(stmp, O_RDWR|O_CREAT, 0644);
		if (lockfd >= 0) {
		    for (err = -1, i = 0; i < (LIBRCC_LOCK_WAIT/10); i++) {
			err = flock(lockfd, LOCK_EX|LOCK_NB);
			if ((err)&&(errno == EWOULDBLOCK)) nanosleep(&wait, NULL);
			else break;
		    }
		    
		    if (err) close(lockfd);
		    else return 0;
		} 
	    }
	    
	    lockfd = -1;
	    return -1;
	} 

	return 0;
    }
    
    return -1;
# else  /* HAVE_SYS_FILE_H */
    return 0;
# endif /* HAVE_SYS_FILE_H */
}

void rccUnLock() {
#ifdef HAVE_SYS_FILE_H
    int size;
    char *stmp;

    if (lockfd<0) return;
    
    size = strlen(rcc_home_dir) + 32;
    stmp = (char*)malloc(size*sizeof(char));
    if (!stmp) return;

    sprintf(stmp,"%s/.rcc/locks/rcc.lock", rcc_home_dir);

    flock(lockfd, LOCK_UN);
    close(lockfd);
    lockfd = -1;
#endif /* HAVE_SYS_FILE_H */
}

rcc_context rccCreateContext(const char *locale_variable, unsigned int max_languages, unsigned int max_classes, rcc_class_ptr defclasses, rcc_init_flags flags) {
    unsigned int i;
    
    rcc_context ctx;
    rcc_language_ptr *languages;
    rcc_language_internal *ilang;
    rcc_class_ptr *classes;
    rcc_class_internal *iclass;
    rcc_language_config configs;
    rcc_iconv *from;
    rcc_mutex mutex;

    if (!initialized) return NULL;
    
    if (!max_languages) {
	if (flags&RCC_FLAG_NO_DEFAULT_CONFIGURATION) max_languages = RCC_MAX_LANGUAGES;
	else {
	    for (i=0;rcc_default_languages[i].sn;i++);
	    max_languages = i;
	}
    }
    
    if (!max_classes) {
	if (defclasses) {
	    for (i=0;defclasses[i].name;i++);
	    max_classes = i;
	} else max_classes = RCC_MAX_CLASSES;
    }

    ctx = (rcc_context)malloc(sizeof(struct rcc_context_t));
    languages = (rcc_language_ptr*)malloc((max_languages+1)*sizeof(rcc_language_ptr));
    classes = (rcc_class_ptr*)malloc((max_classes+1)*sizeof(rcc_class_ptr));
    from = (rcc_iconv*)malloc((max_classes)*sizeof(rcc_iconv));
    ilang = (rcc_language_internal*)malloc((max_languages+1)*sizeof(rcc_language_internal));
    iclass = (rcc_class_internal*)malloc((max_classes+1)*sizeof(rcc_class_internal));
    mutex = rccMutexCreate();

    configs = (rcc_language_config)malloc((max_languages)*sizeof(struct rcc_language_config_t));
    
    if ((!ctx)||(!languages)||(!classes)||(!mutex)||(!from)||(!ilang)||(!iclass)||(!mutex)) {
	if (mutex) rccMutexFree(mutex);
	if (from) free(from);
	if (configs) free(configs);
	if (classes) free(classes);
	if (languages) free(languages);
	if (ilang) free(ilang);
	if (iclass) free(iclass);
	if (ctx) free(ctx);
	return NULL;
    }

    ctx->configuration_lock = 0;
    
    ctx->mutex = mutex;
    
    ctx->db4ctx = NULL;

    ctx->aliases[0] = NULL;
    for (i=0;rcc_default_aliases[i].alias;i++)
	rccRegisterLanguageAlias(ctx, rcc_default_aliases + i);
    
    ctx->ilang = ilang;
    ctx->iclass = iclass;
    
    ctx->languages = languages;
    ctx->max_languages = max_languages;
    ctx->n_languages = 0;
    languages[0] = NULL;

    ctx->classes = classes;
    ctx->max_classes = max_classes;
    ctx->n_classes = 0;
    classes[0] = NULL;

    ctx->lastprefix[0] = 0;
    
    ctx->iconv_from = from;
    for (i=0;i<max_classes;i++) from[i] = NULL;
    for (i=0;i<RCC_MAX_CHARSETS;i++) ctx->iconv_auto[i] = NULL;
    
    ctx->configs = configs;
    for (i=0;i<max_languages;i++)
	configs[i].charset = NULL;

    ctx->current_language = 0;
    ctx->default_language = 0;

    if (locale_variable) {
	if (strlen(locale_variable)>=RCC_MAX_VARIABLE_CHARS) {
	    rccFree(ctx);
	    return NULL;
	}
	strcpy(ctx->locale_variable, locale_variable);
    } else {
	strcpy(ctx->locale_variable, RCC_LOCALE_VARIABLE);
    }

    for (i=0;i<RCC_MAX_OPTIONS;i++) rccOptionSetDefault(ctx, (rcc_option)i);
    
    if (flags&RCC_FLAG_NO_DEFAULT_CONFIGURATION) {
	rccRegisterLanguage(ctx, rcc_default_languages);
	ctx->current_config = NULL;
    } else {
	for (i=0;rcc_default_languages[i].sn;i++)
	    rccRegisterLanguage(ctx, rcc_default_languages+i);

	if (max_languages < i) {
	    rccFree(ctx);
	    return NULL;
	}

	for (i=0;rcc_default_relations[i].lang;i++)
	    rccRegisterLanguageRelation(ctx, rcc_default_relations+i);
	
	ctx->current_config = rccGetCurrentConfig(ctx);
    } 
    
    if (defclasses) {
	for (i=0;defclasses[i].name;i++)
	    rccRegisterClass(ctx, defclasses+i);

	if (max_classes < i) {
	    rccFree(ctx);
	    return NULL;
	}	    
    }

    ctx->configure = 1;
    
    return ctx;
}

int rccInitDefaultContext(const char *locale_variable, unsigned int max_languages, unsigned int max_classes, rcc_class_ptr defclasses, rcc_init_flags flags) {
    if (!initialized) return -1;
    if (rcc_default_ctx) rccFreeContext(rcc_default_ctx);
    rcc_default_ctx = rccCreateContext(locale_variable, max_languages, max_classes, defclasses, flags);
    if (rcc_default_ctx) return 0;
    return -1;
}


static void rccFreeIConv(rcc_context ctx) {
    unsigned int i;
    
    if ((!ctx)||(!ctx->iconv_from)) return;

    for (i=0;i<ctx->n_classes;i++) {
	if (ctx->iconv_from[i]) {
	    rccIConvClose(ctx->iconv_from[i]);
	    ctx->iconv_from[i] = NULL;
	}
    }
    for (i=0;i<RCC_MAX_CHARSETS;i++) {
	if (ctx->iconv_auto[i]) {
	    rccIConvClose(ctx->iconv_auto[i]);
	    ctx->iconv_auto[i] = NULL;
	}
    }    
}

void rccFreeContext(rcc_context ctx) {
    unsigned int i;
    
    if (ctx) {
	if (ctx->db4ctx) rccDb4FreeContext(ctx->db4ctx);
	rccFreeIConv(ctx);
	if (ctx->iconv_from) free(ctx->iconv_from);
	
	if (ctx->configs) {
	    for (i=0;i<ctx->max_languages;i++)
		rccConfigClear(ctx->configs+i);
	    free(ctx->configs);
	}
	if (ctx->iclass) free(ctx->iclass);
	if (ctx->classes) free(ctx->classes);
	if (ctx->ilang) free(ctx->ilang);
	if (ctx->languages) free(ctx->languages);
	if (ctx->mutex) rccMutexFree(ctx->mutex);
	free(ctx);
    }
}

int rccInitDb4(rcc_context ctx, const char *name, rcc_db4_flags flags) {
    size_t size;
    char *dbname;
    
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    if (!name) name = "default";

    size = strlen(rcc_home_dir) + strlen(name) + 32;
    dbname = (char*)malloc(size*sizeof(char));
    if (!dbname) return -1;

    sprintf(dbname,"%s/.rcc/",rcc_home_dir);
    mkdir(dbname, 00755);
    
    sprintf(dbname,"%s/.rcc/%s.db/",rcc_home_dir,name);
    mkdir(dbname, 00755);

    ctx->db4ctx = rccDb4CreateContext(dbname, flags);
    free(dbname);	
    
    if (!ctx->db4ctx) return -1;

    return 0;
}

int rccLockConfiguration(rcc_context ctx, unsigned int lock_code) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    if (ctx->configuration_lock) return -1;
    ctx->configuration_lock = lock_code;
    return 0;
}

int rccUnlockConfiguration(rcc_context ctx, unsigned int lock_code) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    if (ctx->configuration_lock != lock_code) return -1;
    ctx->configuration_lock = 0;
    return 0;    
}

rcc_language_id rccRegisterLanguage(rcc_context ctx, rcc_language *language) {
    unsigned int i;
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_language_id)-1;
    }
    if (!language) return (rcc_language_id)-1;
    if (ctx->configuration_lock) return (rcc_language_id)-1;
    
    if (ctx->n_languages == ctx->max_languages) return (rcc_language_id)-1;
    
    memcpy(ctx->ilang + ctx->n_languages, language, sizeof(rcc_language));
    ctx->ilang[ctx->n_languages].parrents[0] = (rcc_language_id)-1;
    ctx->ilang[ctx->n_languages].latin = 0;
    
    for (i=0;language->charsets[i];i++)
	if ((strstr(language->charsets[i],"8859"))&&(language->charsets[i][strlen(language->charsets[i])-1]=='1')) {
	    ctx->ilang[ctx->n_languages].latin = 1;
	    break;
	}

    if ((i==1)&&(!language->charsets[1])&&(rccIsUTF8(language->charsets[0])))
	    ctx->ilang[ctx->n_languages].latin = 1;

    ctx->languages[ctx->n_languages++] = (rcc_language_ptr)(ctx->ilang + ctx->n_languages);
    ctx->languages[ctx->n_languages] = NULL;
    
    if (!ctx->current_language)
	ctx->current_config = rccGetCurrentConfig(ctx);
    
    return ctx->n_languages-1;
}

rcc_charset_id rccLanguageRegisterCharset(rcc_language *language, rcc_charset charset) {
    unsigned int i;
    
    if ((!language)||(!charset)) return (rcc_charset_id)-1;
    for (i=0;language->charsets[i];i++);
    if (i>=RCC_MAX_CHARSETS) return (rcc_charset_id)-1;

    if ((strstr(charset,"8859"))&&(charset[strlen(charset)-1]=='1')) 
	((rcc_language_internal*)language)->latin = 1;
    
    language->charsets[i++] = charset;
    language->charsets[i] = NULL;
    return i-1;
}

rcc_engine_id rccLanguageRegisterEngine(rcc_language *language, rcc_engine *engine) {
    unsigned int i;
    
    if ((!language)||(!engine)) return (rcc_engine_id)-1;
    for (i=0;language->engines[i];i++);
    if (i>=RCC_MAX_ENGINES) return (rcc_engine_id)-1;
    language->engines[i++] = engine;
    language->engines[i] = NULL;
    return i-1;
}

rcc_alias_id rccRegisterLanguageAlias(rcc_context ctx, rcc_language_alias *alias) {
    unsigned int i;
    
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_alias_id)-1;
    }
    if (!alias) return (rcc_alias_id)-1;
    
    for (i=0;ctx->aliases[i];i++)
    if (i>=RCC_MAX_ALIASES) return (rcc_alias_id)-1;
    
    ctx->aliases[i++] = alias;
    ctx->aliases[i] = NULL;

    return i-1;
}

rcc_relation_id rccRegisterLanguageRelation(rcc_context ctx, rcc_language_relation *relation) {
    unsigned int i;
    rcc_language_id language_id;
    const char *lang;
    const char *parrent;
    rcc_language_id *list;
    
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_alias_id)-1;
    }
    if (!relation) return (rcc_relation_id)-1;

    lang = relation->lang;
    parrent = relation->parrent;
    if ((!lang)||(!parrent)||(!strcasecmp(lang,parrent))) return (rcc_relation_id)-1;
    
    language_id = rccGetLanguageByName(ctx, lang);
    if (language_id == (rcc_language_id)-1) return (rcc_relation_id)-1;
    

    list = ((rcc_language_internal*)ctx->languages[language_id])->parrents;

    language_id = rccGetLanguageByName(ctx, parrent);
    if (language_id == (rcc_language_id)-1) return (rcc_relation_id)0;
    
    for (i=0;list[i]!=(rcc_language_id)-1;i++)
	if (list[i] == language_id) return (rcc_relation_id)0;

    if (i<RCC_MAX_LANGUAGE_PARRENTS) {
	list[i++] = language_id;
    	list[i] = (rcc_language_id)-1;
    } else return (rcc_relation_id)-1;
    
    
    return (rcc_relation_id)0;
}


rcc_class_id rccRegisterClass(rcc_context ctx, rcc_class *cl) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_class_id)-1;
    }
    if (!cl) return (rcc_class_id)-1;
    if (ctx->configuration_lock) return (rcc_class_id)-1;
    if (ctx->n_classes == ctx->max_classes) return (rcc_class_id)-1;

    ctx->configure = 1;
    
    memcpy(ctx->iclass + ctx->n_classes, cl, sizeof(rcc_class));
    ctx->iclass[ctx->n_classes].disabled = NULL;
    ctx->iclass[ctx->n_classes].additional = NULL;

    ctx->classes[ctx->n_classes++] = (rcc_class_ptr)(ctx->iclass + ctx->n_classes);
    ctx->classes[ctx->n_classes] = NULL;
    
    if (!strcasecmp(cl->name, "id3")) {
	rccRegisterDisabledCharsets(ctx, ctx->n_classes - 1, rcc_default_disabled_id3_charsets);
    } else if (!strcasecmp(cl->name, "id3v2")) {
	rccRegisterAdditionalCharsets(ctx, ctx->n_classes - 1, rcc_default_additional_id3v2_charsets);
    }

    return ctx->n_classes-1;
}

int rccRegisterDisabledCharsets(rcc_context ctx, rcc_class_id class_id, rcc_charset *charsets) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    if (ctx->configuration_lock) return -1;
    if ((class_id == (rcc_class_id)-1)||(class_id >= ctx->n_classes)) return -1;

    ctx->iclass[class_id].disabled = charsets;
    return 0;
}

int rccRegisterAdditionalCharsets(rcc_context ctx, rcc_class_id class_id, rcc_charset *charsets) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    if (ctx->configuration_lock) return -1;
    if ((class_id == (rcc_class_id)-1)||(class_id >= ctx->n_classes)) return -1;

    ctx->iclass[class_id].additional = charsets;
    return 0;
}

rcc_class_type rccGetClassType(rcc_context ctx, rcc_class_id class_id) {
    rcc_class_type clt;
    
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return RCC_CLASS_INVALID;
    }

    if ((class_id<0)||(class_id>=ctx->n_classes)) return RCC_CLASS_INVALID;
    
    /*DS: temporary solution */
    
    clt = ctx->classes[class_id]->class_type;
    
    if ((!strcasecmp(ctx->classes[class_id]->name, "out"))&&(clt == RCC_CLASS_STANDARD))
	clt = RCC_CLASS_TRANSLATE_LOCALE;
    
    return clt;
}

const char *rccGetClassName(rcc_context ctx, rcc_class_id class_id) {
    if ((class_id<0)||(class_id>=ctx->n_classes)) return NULL;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return ctx->classes[class_id]->name;
}

const char *rccGetClassFullName(rcc_context ctx, rcc_class_id class_id) {
    if ((class_id<0)||(class_id>=ctx->n_classes)) return NULL;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return NULL;
    }

    return ctx->classes[class_id]->fullname;
}


int rccIsDisabledCharsetName(rcc_context ctx, rcc_class_id class_id, const char *charset) {
    unsigned int i;
    rcc_charset *charsets;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return RCC_CLASS_INVALID;
    }

    if ((!charset)||(class_id<0)||(class_id>=ctx->n_classes)) return -1;

    charsets = ctx->iclass[class_id].disabled;
    if (!charsets) return 0;
    
    for (i=0;charsets[i];i++) {
	if (!strcasecmp(charsets[i], charset)) return 1;
	else if (!strcasecmp(charsets[i], rcc_default_unicode_charsets)) {
	    if (rccIsUnicode(charset)) return 1;
	}
	else if (!strcasecmp(charsets[i], rcc_default_nonunicode_charsets)) {
	    if (!rccIsUnicode(charset)) return 1;
	}
    }
    return 0;    
}


int rccConfigure(rcc_context ctx) {
    unsigned int i;
    rcc_charset *charsets;
    const char *charset;
    rcc_language_config cfg;
    
    if (!ctx) return -1;
    if (!ctx->configure) return 0;
    
    

    cfg = rccGetCurrentConfig(ctx);
    if (!cfg) return -1;

    rccMutexLock(ctx->mutex);
    rccMutexLock(cfg->mutex);

    rccFreeIConv(ctx);
    for (i=0;i<ctx->n_classes;i++) {
	charset = rccConfigGetCurrentCharsetName(cfg, (rcc_class_id)i);
	if ((!charset)||(rccIsUTF8(charset))) continue;
	ctx->iconv_from[i] = rccIConvOpen("UTF-8", charset);
    }
    
    charsets = rccGetCurrentAutoCharsetList(ctx);
    if (charsets) {
	for (i=0;charsets[i];i++) {
	    charset = charsets[i];
	    if ((!charset)||(rccIsUTF8(charset))) continue;
	    ctx->iconv_auto[i] = rccIConvOpen("UTF-8", charset);
	}
    }
    
    ctx->configure = 0;
    
    rccMutexUnLock(cfg->mutex);
    rccMutexUnLock(ctx->mutex);
    
    return 0;
}

char *rccCreateResult(rcc_context ctx, size_t len) {
    char *res;

    if (!len) len = strlen(ctx->tmpbuffer);

    res = (char*)malloc(len+1);
    if (!res) return NULL;

    memcpy(res, ctx->tmpbuffer, len);
    res[len] = 0;
    
    return res;    
}
