#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "rccconfig.h"
#include "rccexternal.h"
#include "rccmutex.h"
#include "rcctranslate.h"

#define RCC_TRANSLATE_DEFAULT_TIMEOUT	1000000	/* 1s */

int rccTranslateInit() {

    return 0;
}

void rccTranslateFree() {
}


rcc_translate rccTranslateOpen(const char *from, const char *to) {
#ifdef HAVE_LIBTRANSLATE
    rcc_translate translate;

    if ((!from)||(!to)||(!strcasecmp(from,to))) return NULL;
    if ((strlen(from)!=2)||(strlen(to)!=2)) return NULL;
    
    translate = (rcc_translate)malloc(sizeof(rcc_translate_s));
    if (!translate) return NULL;
    
    translate->mutex = rccMutexCreate();
    translate->wmutex = rccMutexCreate();
    if ((!translate->mutex)||(!translate->wmutex)) {
	if (translate->mutex) rccMutexFree(translate->mutex);
	if (translate->wmutex) rccMutexFree(translate->wmutex);
	free(translate);
	return NULL;
    }

    translate->sock = rccExternalConnect(RCC_EXTERNAL_MODULE_LIBRTRANSLATE);
    if (translate->sock == -1) {
	rccMutexFree(translate->mutex);
	rccMutexFree(translate->wmutex);
	free(translate);
	return NULL;
    }

    translate->remaining = 0;
    translate->werror = 0;

    translate->prefix.cmd.cmd = RCC_EXTERNAL_COMMAND_TRANSLATE;
    translate->prefix.cmd.size = sizeof(rcc_translate_prefix_s);
    memcpy(translate->prefix.from, from, 3*sizeof(char));
    memcpy(translate->prefix.to, to,  3*sizeof(char));

    translate->wprefix.cmd.cmd = RCC_EXTERNAL_COMMAND_TRANSLATE_QUEUE;
    translate->wprefix.cmd.size = sizeof(rcc_translate_prefix_s);
    memcpy(translate->wprefix.from, from, 3*sizeof(char));
    memcpy(translate->wprefix.to, to,  3*sizeof(char));

    rccTranslateSetTimeout(translate, RCC_TRANSLATE_DEFAULT_TIMEOUT);

    return translate;
#else
    return NULL;
#endif /* HAVE_LIBTRANSLATE */
}

void rccTranslateClose(rcc_translate translate) {
#ifdef HAVE_LIBTRANSLATE
    if (!translate) return;
    if (translate->sock != -1) rccExternalClose(translate->sock);
    rccMutexFree(translate->mutex);
    rccMutexFree(translate->wmutex);
    free(translate);
#endif /* HAVE_LIBTRANSLATE */
}

int rccTranslateSetTimeout(rcc_translate translate, unsigned long us) {
    if (!translate) return -1;
    translate->prefix.timeout = us;
    return 0;
}

#define RCC_UNLOCK_W 1
#define RCC_UNLOCK_R 2
#define RCC_UNLOCK_RW 3
#define RCC_UNLOCK_WR 3
static char *rccTranslateReturn(rcc_translate translate, char *ret, int unlock) {
    if (unlock&RCC_UNLOCK_R) rccMutexUnLock(translate->mutex);
    if (unlock&RCC_UNLOCK_W) rccMutexUnLock(translate->wmutex);
    return ret;
}
#define rccTranslateReturnNULL(translate, unlock) rccTranslateReturn(translate, NULL, unlock) 

static int rccTranslateQueue(rcc_translate translate, const char *buf) {
    size_t len, err;
    
    
    len = strlen(buf);
    translate->wprefix.cmd.size = sizeof(rcc_translate_prefix_s) + len - sizeof(rcc_external_command_s);

    err = rccExternalWrite(translate->sock, (char*)&translate->wprefix, sizeof(rcc_translate_prefix_s) - 1, 0);
    if (!err) err = rccExternalWrite(translate->sock, buf, len + 1, 0);
    return err?1:0;
}

char *rccTranslate(rcc_translate translate, const char *buf) {
#ifdef HAVE_LIBTRANSLATE
    rcc_external_command_s resp;
    size_t err, len;
    char *buffer;
    size_t i;

    if ((!translate)||(!buf)) return NULL;

    if (!strcmp(translate->prefix.to, rcc_english_language_sn)) {
	for (i=0;buf[i];i++) {
	    if ((unsigned char)buf[i]>0x7F) break;
	    if ((buf[i]>='A')&&(buf[i]<='Z')) break;
	    if ((buf[i]>='a')&&(buf[i]<='z')) break;
	}
	if (!buf[i]) return NULL;
    }
    
    rccMutexLock(translate->wmutex);
    
    if (rccMutexTryLock(translate->mutex)) {
	if ((translate->werror)||(translate->sock == -1)) return rccTranslateReturnNULL(translate,RCC_UNLOCK_W);

	if (rccTranslateQueue(translate, buf)) translate->werror = 1;
	return rccTranslateReturnNULL(translate, RCC_UNLOCK_W);
    }
    
    if (translate->werror) {
	rccExternalClose(translate->sock);
	translate->sock = -1;
	translate->werror = 0;
    }
    
    if (translate->sock == -1) {
	translate->sock = rccExternalConnect(RCC_EXTERNAL_MODULE_LIBRTRANSLATE);
	if (translate->sock == -1) {
	    return rccTranslateReturnNULL(translate,RCC_UNLOCK_RW);
	} else {
	    translate->werror = 0;
	    translate->remaining = 0;
	}
    } else if (translate->remaining) {
	if (translate->remaining == (size_t)-1) {
	    err = rccExternalRead(translate->sock, (char*)&resp, sizeof(rcc_external_command_s), 0);
	    if (err) {
		if (err == sizeof(rcc_external_command_s)) {
		    if (rccTranslateQueue(translate, buf)) {
		        rccExternalClose(translate->sock);
			translate->sock = -1;
		    }
		} else {
		    rccExternalClose(translate->sock);
		    translate->sock = -1;
		}
		return rccTranslateReturnNULL(translate,RCC_UNLOCK_RW);
	    }
	    translate->remaining = resp.size;
	}
	
	buffer = (char*)malloc(translate->remaining*sizeof(char));
	if (!buffer) {
	    rccExternalClose(translate->sock);
	    translate->sock = -1;
	    return rccTranslateReturnNULL(translate,RCC_UNLOCK_RW);
	}
	
	err = rccExternalRead(translate->sock, buffer, translate->remaining, 0);
	free(buffer);
	if (err) {
	    translate->remaining = err;
	    if (rccTranslateQueue(translate, buf)) {
		rccExternalClose(translate->sock);
		translate->sock = -1;
	    }
	    return rccTranslateReturnNULL(translate,RCC_UNLOCK_RW);
	}
	translate->remaining = 0;
    }
    
    len = strlen(buf);
    translate->prefix.cmd.size = sizeof(rcc_translate_prefix_s) + len - sizeof(rcc_external_command_s);
    err = rccExternalWrite(translate->sock, (char*)&translate->prefix, sizeof(rcc_translate_prefix_s) - 1, 0);
    if (err) {
	rccExternalClose(translate->sock);
	translate->sock = -1;
	return rccTranslateReturnNULL(translate,RCC_UNLOCK_RW);
    }
    err = rccExternalWrite(translate->sock, buf, len + 1, 0);
    if (err) {
	rccExternalClose(translate->sock);
	translate->sock = -1;
	return rccTranslateReturnNULL(translate,RCC_UNLOCK_RW);
    }
    rccMutexUnLock(translate->wmutex);

    err = rccExternalRead(translate->sock, (char*)&resp, sizeof(rcc_external_command_s), translate->prefix.timeout);
    if (err) {
	if (err == sizeof(rcc_external_command_s)) {
	    translate->remaining = (size_t)-1;
	} else {
	    rccMutexLock(translate->wmutex);
	    rccExternalClose(translate->sock);
	    translate->sock = -1;
	    rccMutexUnLock(translate->wmutex);
	}
	return rccTranslateReturnNULL(translate,RCC_UNLOCK_R);
    }

    if ((resp.cmd!=RCC_EXTERNAL_COMMAND_TRANSLATE)||(!resp.size))
	return rccTranslateReturnNULL(translate,RCC_UNLOCK_R);
    
    buffer = (char*)malloc(resp.size*sizeof(char));
    if (!buffer) {
	rccMutexLock(translate->wmutex);
	rccExternalClose(translate->sock);
	translate->sock = -1;
	rccMutexUnLock(translate->wmutex);

	return rccTranslateReturnNULL(translate,RCC_UNLOCK_R);
    }

    err = rccExternalRead(translate->sock, buffer, resp.size, 0);
    if (err) {
	translate->remaining = err;
	free(buffer);
	return rccTranslateReturnNULL(translate,RCC_UNLOCK_R);
    }
    
    return rccTranslateReturn(translate, buffer, RCC_UNLOCK_R);
#else
    return NULL;
#endif /* HAVE_LIBTRANSLATE */
}
