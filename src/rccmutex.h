#ifndef _RCC_MUTEX_H
#define _RCC_MUTEX_H

#include "../config.h"

#ifdef HAVE_PTHREAD
# include <pthread.h>
#endif /* HAVE_PTHREAD */

struct rcc_mutex_t {
#ifdef HAVE_PTHREAD
    pthread_mutex_t mutex;
#else
    unsigned char mutex;
#endif /* HAVE_PTHREAD */
};
typedef struct rcc_mutex_t rcc_mutex_s;
typedef struct rcc_mutex_t *rcc_mutex;

rcc_mutex rccMutexCreate();
void rccMutexFree(rcc_mutex mutex);

int rccMutexLock(rcc_mutex mutex);
int rccMutexTryLock(rcc_mutex mutex);
void rccMutexUnLock(rcc_mutex mutex);
 
#endif /* _RCC_MUTEX_H */
