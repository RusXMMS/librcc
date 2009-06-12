/*
  LibRCC - external module interfacying libtranslate library

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

#ifdef HAVE_LIBTRANSLATE
#include <translate.h>
#endif /* HAVE_LIBTRANSLATE */

#include "../src/rccexternal.h"
#include "../src/rcctranslate.h"
#include "../src/rccdb4.h"
#include "rcclibtranslate.h"


#ifdef HAVE_LIBTRANSLATE
static TranslateSession *session = NULL;
static db4_context db4ctx = NULL;

static int exitflag = 0;
static GMutex *mutex = NULL;
static GCond *cond = NULL;
static GQueue *queue = NULL;
static GThread *thread = NULL;

extern char rcc_external_offline;

static char *rccCreateKey(const char *from, const char *to, const char *data, size_t *keysize) {
    char *res;

    *keysize = strlen(data) + 4;
    res = (char*)malloc((*keysize+1)*sizeof(char));
    if (res) sprintf(res,"%s%s%s",from,to,data);
    return res;
}

static char *rccTranslateFixEOL(char *result, const char *text) {
    size_t i,j;
    char *res;
    
    if (!result) return result;
    if (strstr(text, "\r\n")) return result;
    
    res = (char*)malloc((strlen(result)+1)*sizeof(char));
    if (!res) {
	free(result);
	return NULL;
    }
    
    for (i=0, j=0;result[i];i++) {
	if ((result[i]=='\r')&&(result[i+1]=='\n')) i++;
	else res[j++] = result[i];
    }
    res[j] = 0;
    free(result);
    return res;
}

static void *rccLibPostponed(void *info) {
    char *result;
    char *data;
    char from[3];
    char to[3];
    size_t datalen;
    
    from[2] = 0;
    to[2] = 0;
    
    g_mutex_lock(mutex);
    while ((!exitflag)||(rcc_external_offline)) {
	data = (char*)g_queue_pop_head(queue);
	if (data) {
	    g_mutex_unlock(mutex);
	    
	    datalen = strlen(data);
	    
	    memcpy(from, data, 2);
	    memcpy(to, data + 2, 2);

	    result = rccDb4GetKey(db4ctx, data, datalen);
	    if (result) free(result);
	    else {
		result = translate_session_translate_text(session, data + 4, from, to, NULL, NULL, NULL);

		if (result) {
		    result = rccTranslateFixEOL(result, data+4);
		    rccDb4SetKey(db4ctx, data, datalen, result);
		    free(result);
		}
	    }

	    free(data);
	    g_mutex_lock(mutex);
	} else {
	    if (exitflag) break;
	    g_cond_wait(cond, mutex);
	}
    }
    g_mutex_unlock(mutex);
    return NULL;
}

#endif /* HAVE_LIBTRANSLATE */


int rccLibTranslateInit(const char *rcc_home_dir) {
#ifdef HAVE_LIBTRANSLATE
    size_t size;
    char *dbname;
    GSList *list = NULL;

    const char *http_proxy;

    if (!translate_init(NULL)) return -1;
    
    http_proxy = getenv("HTTP_PROXY");
    if (!http_proxy) http_proxy = getenv("http_proxy");
    translate_set_proxy(http_proxy);

    list = translate_get_services();

    session = translate_session_new(list);

    g_slist_foreach(list, (GFunc) g_object_unref, NULL);
    g_slist_free(list);
    

    size = strlen(rcc_home_dir) + 32;
    dbname = (char*)malloc(size*sizeof(char));
    if (dbname) {
        sprintf(dbname,"%s/.rcc/",rcc_home_dir);
	mkdir(dbname, 00755);
    
	sprintf(dbname,"%s/.rcc/libtranslate.db/",rcc_home_dir);
	mkdir(dbname, 00755);

	db4ctx = rccDb4CreateContext(dbname, 0);
	free(dbname);
    }
    if (db4ctx) {
	mutex = g_mutex_new();
	cond = g_cond_new();
	if ((mutex)&&(cond))
	    queue = g_queue_new();
	if (queue)
	    thread = g_thread_create(rccLibPostponed, NULL, TRUE, NULL);
    }
#endif /* HAVE_LIBTRANSLATE */

    return 0;
}

void rccLibTranslateFree() {
#ifdef HAVE_LIBTRANSLATE
    char *ptr;
    if  (session) {
	if (thread) {
	    exitflag = 1;
	    g_mutex_lock(mutex);
	    g_cond_signal(cond);
	    g_mutex_unlock(mutex);
	    g_thread_join(thread);
	    thread = NULL;
	    exitflag = 0;
	}
	if (queue) {
	    do {
		ptr = g_queue_pop_head(queue);
		if (ptr) free(ptr);
	    } while (ptr);
	    g_queue_free(queue);
	    queue = NULL;
	}
	if (mutex) {
	    g_mutex_free(mutex);
	    mutex = NULL;
	}
	if (cond) {
	    g_cond_free(cond);
	    cond = NULL;
	}
	if (db4ctx) rccDb4FreeContext(db4ctx);
	g_object_unref(session);
	session = NULL;
    }
#endif /* HAVE_LIBTRANSLATE */
}


static void rccLibTranslateQueue(const char *from, const char *to, const char *text) {
#ifdef HAVE_LIBTRANSLATE
    char *key = NULL;
    size_t keysize;
    
    if ((!session)||(!from)||(!to)||(!text)) return;
    if ((strlen(from)!=2)||(strlen(to)!=2)) return;

    if (db4ctx) {
	key = rccCreateKey(from,to,text,&keysize);
	if (key) {
	    g_mutex_lock(mutex);
	    g_queue_push_tail(queue, key);
	    g_mutex_unlock(mutex);
	    g_cond_signal(cond);
	}
    }
#endif /* HAVE_LIBTRANSLATE */
}

static char *rccLibTranslateDo(const char *from, const char *to, const char *text, unsigned long timeout) {
#ifdef HAVE_LIBTRANSLATE
    char *result;
    char *key = NULL;
    size_t keysize;
    
    if ((!session)||(!from)||(!to)||(!text)) return NULL;
    if ((strlen(from)!=2)||(strlen(to)!=2)) return NULL;

    if (db4ctx) {
	key = rccCreateKey(from,to,text,&keysize);
	if (key) {
	    result = rccDb4GetKey(db4ctx, key, keysize);
	    if (result) {
		free(key);
		return result;
	    }
	}
    }
# ifdef HAVE_LIBTRANSLATE_TIMED_TRANSLATE    
    result = translate_session_timed_translate_text(session, text, from, to, timeout, NULL, NULL, NULL);
# else
    result = translate_session_translate_text(session, text, from, to, NULL, NULL, NULL);
# endif /* HAVE_LIBTRANSLATE_TIMED_TRANSLATE */

    result = rccTranslateFixEOL(result, text);
    
    if ((db4ctx)&&(key)) {
	if (result) {
	    rccDb4SetKey(db4ctx, key, keysize, result);
	    free(key);
	}
	else {
	    g_mutex_lock(mutex);
	    g_queue_push_tail(queue, key);
	    g_mutex_unlock(mutex);
	    g_cond_signal(cond);
	}
    }

    return result;
#else
    return NULL;
#endif /* HAVE_LIBTRANSLATE */
}


void *rccLibTranslate(void *info) {
    int s;
#ifdef HAVE_LIBTRANSLATE
    unsigned char connected = 1;
    rcc_translate_prefix_s prefix;
    ssize_t readed, writed, res;
    char *buffer;
    size_t size;
    char *translated = NULL;
#endif /* HAVE_LIBTRANSLATE */

    if (!info) return NULL;
    
    s = ((rcc_external_info)info)->s;
    free(info);
    
#ifdef HAVE_LIBTRANSLATE
    while (connected) {
	readed = read(s, &prefix, sizeof(rcc_translate_prefix_s)-1);
	if ((readed<=0)||(readed != (sizeof(rcc_translate_prefix_s)-1))) break;

	switch (prefix.cmd.cmd) {
	    case RCC_EXTERNAL_COMMAND_CLOSE:
		connected = 0;
	    break;
	    case RCC_EXTERNAL_COMMAND_TRANSLATE:
		size = 1 + prefix.cmd.size + sizeof(rcc_external_command_s) - sizeof(rcc_translate_prefix_s);
		buffer = (char*)malloc(size);
		if (buffer) {
		    for (readed = 0; (readed < size)&&(connected); readed += res) {
			res = read(s, buffer + readed, size - readed);
			if (res<=0) connected = 0;
		    }
		    if (!connected)  goto clear;
		    
		    prefix.cmd.cmd = 0;
		    prefix.cmd.size = 0;
		    
		    if ((prefix.from[2])||(prefix.to[2])||(buffer[readed-1])) goto respond;
		    
		    translated = rccLibTranslateDo(prefix.from, prefix.to, buffer, prefix.timeout);
		    if (translated) {
			prefix.cmd.cmd = RCC_EXTERNAL_COMMAND_TRANSLATE;
			prefix.cmd.size = strlen(translated) + 1;
		    }
		    
respond:
		    res = write(s, &prefix.cmd, sizeof(rcc_external_command_s));
		    if (res == sizeof(rcc_external_command_s)) {
			for (writed = 0; (writed < prefix.cmd.size)&&(connected); writed += res) {
			    res = write(s, translated + writed, prefix.cmd.size - writed);
			    if (res<=0) connected = 0;
			}
		    } else connected = 0;
		    
		    if (prefix.cmd.size) free(translated);		    
clear:
		    free(buffer);
		} else connected = 0;
	    break;
	    case RCC_EXTERNAL_COMMAND_TRANSLATE_QUEUE:
		size = 1 + prefix.cmd.size + sizeof(rcc_external_command_s) - sizeof(rcc_translate_prefix_s);
		buffer = (char*)malloc(size);
		if (buffer) {
		    for (readed = 0; (readed < size)&&(connected); readed += res) {
			res = read(s, buffer + readed, size - readed);
			if (res<=0) connected = 0;
		    }
		    if ((connected)&&(!prefix.from[2])&&(!prefix.to[2])&&(!buffer[readed-1])) {
			rccLibTranslateQueue(prefix.from, prefix.to, buffer);
		    }
		    free(buffer);
		} else connected = 0;
	    break;
	    default:
		size = 1 + prefix.cmd.size + sizeof(rcc_external_command_s) - sizeof(rcc_translate_prefix_s);
		buffer = (char*)malloc(size);
		if (buffer) {
		    for (readed = 0; (readed < size)&&(connected); readed += res) {
			res = read(s, buffer + readed, size - readed);
			if (res<=0) connected = 0;
		    }
		    free(buffer);
		} else connected = 0;
	}
    }    
#endif /* HAVE_LIBTRANSLATE */
    
    close(s);
    return NULL;
}

