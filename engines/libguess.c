#include <stdio.h>
#include <string.h>

#include <librcc.h>
#include <libguess.h>

#define UTF8_ID 0
#define UTF16_ID 1

typedef const char *(*guess_function)(const char *buf, int buflen);

struct rcc_guess_engine_t {
    struct rcc_engine_t engine;
    guess_function func;
};
typedef struct rcc_guess_engine_t rcc_guess_engine;

rcc_autocharset_id guessDetect(rcc_engine_context ctx, const char *buf, int len) {
    const char *res;
    rcc_guess_engine *info;

    if (!buf) return (rcc_autocharset_id)-1;
    
    info = (rcc_guess_engine*)rccEngineGetInfo(ctx);
    if (info) {
	if (info->func) res = info->func(buf, len?len:strlen(buf));
	else {
	    if (!len) len = strlen(buf);
	    res = guess_cn(buf, len);
	    if (!res) res = guess_tw(buf, len);
	    printf("%s\n",res?res:"null");
	}
    } else 
	res = NULL;
	
    if (!res) return (rcc_autocharset_id)-1;
    return rccEngineGetAutoCharsetByName(ctx, res);
}

    
struct rcc_guess_engine_t guessJPEngine = {
 {
    "LibGUESS",
    NULL,		/* Constructor */
    NULL, 		/* Destructor */
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-JP", "EUC-JP", "SJIS", NULL}
 },
 &guess_jp
};

struct rcc_guess_engine_t guessZHEngine = {
 {
    "LibGUESS",
    NULL,		/* Constructor */
    NULL, 		/* Destructor */
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-CN", "GB2312", "GB18030", "BIG5", NULL}
 },
 NULL
};

/*
struct rcc_guess_engine_t guessCNEngine = {
 {
    "LibGUESS",
    NULL,
    NULL,
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-CN", "GB2312", "GB18030", NULL}
 },
 &guess_cn
};

struct rcc_guess_engine_t guessTWEngine = {
 {
    "LibGUESS",
    NULL,
    NULL,
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-TW", "BIG5", NULL}
 },
 &guess_tw
};
*/

struct rcc_guess_engine_t guessKREngine = {
 {
    "LibGUESS",
    NULL,		/* Constructor */
    NULL, 		/* Destructor */
    &guessDetect,
    {"UTF-8", "UTF-16", "ISO-2022-KR", "EUC-KR", "JOHAB", NULL}
 },
 &guess_kr
};
        
    
rcc_engine *rccGetInfo(const char *lang) {
    if (!strcmp(lang, "zh")) return (rcc_engine*)&guessZHEngine;
    if (!strcmp(lang, "ja")) return (rcc_engine*)&guessJPEngine;
    if (!strcmp(lang, "ko")) return (rcc_engine*)&guessKREngine;
    return NULL;
}
