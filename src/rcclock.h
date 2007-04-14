#ifndef _RCC_LOCK_H
#define _RCC_LOCK_H

#ifndef LIBRCC_LOCK_WAIT
# define LIBRCC_LOCK_WAIT 3000 /* ms */
#endif /* LIBRCC_LOCK_WAIT */

int rccLock();
void rccUnLock();

#endif /* _RCC_LOCK_H */

