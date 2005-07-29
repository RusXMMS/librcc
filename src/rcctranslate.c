#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "rccexternal.h"
#include "rcctranslate.h"



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

    translate->sock = rccExternalConnect(RCC_EXTERNAL_MODULE_LIBRTRANSLATE);
    if (translate->sock == -1) {
	free(translate);
	return NULL;
    }

    translate->remaining = 0;
    translate->prefix.cmd.cmd = RCC_EXTERNAL_COMMAND_TRANSLATE;
    translate->prefix.cmd.size = sizeof(rcc_translate_prefix_s);
    memcpy(translate->prefix.from, from, 3*sizeof(char));
    memcpy(translate->prefix.to, to,  3*sizeof(char));
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
    free(translate);
#endif /* HAVE_LIBTRANSLATE */
}

int rccTranslateSetTimeout(rcc_translate translate, unsigned long us) {
#ifdef HAVE_LIBTRANSLATE_TIMED_TRANSLATE    
    if (!translate) return -1;
    translate->prefix.timeout = us;
    return 0;
#else
    return -1;
#endif /* HAVE_LIBTRANSLATE_TIMED_TRANSLATE */
}

char *rccTranslate(rcc_translate translate, const char *buf) {
#ifdef HAVE_LIBTRANSLATE
    size_t i;
    rcc_external_command_s resp;
    size_t err, len;
    char *buffer;

    if ((!translate)||(!buf)) return NULL;
    
    if (!strcmp(translate->prefix.to, "en")) {
	for (i=0;buf[i];i++) 
	    if ((unsigned char)buf[i]>0x7F) break;
	if (!buf[i]) return NULL;
    }
    
    if (translate->sock == -1) {
	translate->sock = rccExternalConnect(RCC_EXTERNAL_MODULE_LIBRTRANSLATE);
	if (translate->sock == -1) return NULL;
    } else if (translate->remaining) {
	if (translate->remaining == (size_t)-1) {
	    err = rccExternalRead(translate->sock, (char*)&resp, sizeof(rcc_external_command_s), 0);
	    if (err) return NULL;
	    translate->remaining = resp.size;
	}
	
	buffer = (char*)malloc(translate->remaining*sizeof(char));
	if (!buffer) {
	    rccExternalClose(translate->sock);
	    translate->sock = -1;
	    return NULL;
	}
	err = rccExternalRead(translate->sock, buffer, translate->remaining, 0);
	free(buffer);
	if (err) {
	    translate->remaining = err;
	    return NULL;
	}
	translate->remaining = 0;
    }
    
    len = strlen(buf);
    translate->prefix.cmd.size = sizeof(rcc_translate_prefix_s) + len - sizeof(rcc_external_command_s);
    err = rccExternalWrite(translate->sock, (char*)&translate->prefix, sizeof(rcc_translate_prefix_s) - 1, 0);
    if (err) {
	rccExternalClose(translate->sock);
	translate->sock = -1;
	return NULL;
    }
    err = rccExternalWrite(translate->sock, buf, len + 1, 0);
    if (err) {
	rccExternalClose(translate->sock);
	translate->sock = -1;
	return NULL;
    }

    err = rccExternalRead(translate->sock, (char*)&resp, sizeof(rcc_external_command_s), translate->prefix.timeout);
    if (err) {
	if (err == sizeof(rcc_external_command_s)) {
	    translate->remaining = (size_t)-1;
	} else {
	    rccExternalClose(translate->sock);
	    translate->sock = -1;
	}
	return NULL;
    }
    if ((resp.cmd!=RCC_EXTERNAL_COMMAND_TRANSLATE)||(!resp.size)) return NULL;
    
    buffer = (char*)malloc(resp.size*sizeof(char));
    if (!buffer) {
	rccExternalClose(translate->sock);
	translate->sock = -1;
	return NULL;
    }
    err = rccExternalRead(translate->sock, buffer, resp.size, 0);
    if (err) {
	translate->remaining = err;
	free(buffer);
	return NULL;
    }
    
    return buffer;
#else
    return NULL;
#endif /* HAVE_LIBTRANSLATE */
}
