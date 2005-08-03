#include <stdlib.h>
#include <time.h>

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
    struct timespec ts;
#endif /* !HAVE_PTHREAD */

    if (!mutex) return -1;
    
#ifdef HAVE_PTHREAD
    return pthread_mutex_lock(&mutex->mutex);
#else
    while (mutex->mutex) {
	    ts.tv_sec = RCC_MUTEX_SLEEP / 1000000;
	    ts.tv_nsec = (RCC_MUTEX_SLEEP % 1000000)*1000;
	    nanosleep(&ts, NULL);
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


