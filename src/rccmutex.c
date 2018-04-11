/*
  LibRCC - module responsible for thread synchronization

  Copyright (C) 2005-2018 Suren A. Chilingaryan <csa@suren.me>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdlib.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef WIN32
# include <windows.h>
#endif /* WIN32 */

#include "rccmutex.h"


#define RCC_MUTEX_SLEEP 500

rcc_mutex rccMutexCreate() {
    rcc_mutex mutex;
    
    mutex = (rcc_mutex)malloc(sizeof(rcc_mutex_s));
    if (mutex) {
#ifdef HAVE_PTHREAD
	pthread_mutex_init(&mutex->mutex, NULL);
#else
	mutex->mutex = 0;
#endif /* HAVE_PTHREAD */
    }
    return mutex;
}

void rccMutexFree(rcc_mutex mutex) {
    if (mutex) {
#ifdef HAVE_PTHREAD
	pthread_mutex_destroy(&mutex->mutex);
#endif /* HAVE_PTHREAD */
	free(mutex);
    }
}

int rccMutexLock(rcc_mutex mutex) {
#ifndef HAVE_PTHREAD
# ifdef HAVE_NANOSLEEP
    struct timespec ts;
# endif /* HAVE_NANOSLEEP */
#endif /* !HAVE_PTHREAD */

    if (!mutex) return -1;
    
#ifdef HAVE_PTHREAD
    return pthread_mutex_lock(&mutex->mutex);
#else
    while (mutex->mutex) {
# if defined(HAVE_NANOSLEEP)
	    ts.tv_sec = RCC_MUTEX_SLEEP / 1000000;
	    ts.tv_nsec = (RCC_MUTEX_SLEEP % 1000000)*1000;
	    nanosleep(&ts, NULL);
# elif defined (HAVE_USLEEP)
	    usleep(RCC_MUTEX_SLEEP);
# elif defined (WIN32)
	    Sleep((RCC_MUTEX_SLEEP<1000)?1:RCC_MUTEX_SLEEP/1000);
# endif /* HAVE_NANOSLEEP */
    }
    mutex->mutex = 1;

    return 0;
#endif /* HAVE_PTHREAD */
}

int rccMutexTryLock(rcc_mutex mutex) {
    if (!mutex) return -1;
    
#ifdef HAVE_PTHREAD
    return pthread_mutex_trylock(&mutex->mutex);
#else
    if (mutex->mutex) return -1;
    mutex->mutex = 1;
    return 0;
#endif /* HAVE_PTHREAD */
}

void rccMutexUnLock(rcc_mutex mutex) {
    if (!mutex) return;
#ifdef HAVE_PTHREAD
    pthread_mutex_unlock(&mutex->mutex);
#else
    mutex->mutex = 0;
#endif /* HAVE_PTHREAD */
}


