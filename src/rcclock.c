/*
  LibRCC - module responsible for application synchronization (write access to
  a shared config mainly)

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "../config.h"

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif /* HAVE_SYS_FILE_H */

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#include "rcchome.h"
#include "rcclock.h"

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
#ifdef _WIN32
    mkdir(stmp);
#else
    mkdir(stmp, 00755);
#endif
    
    sprintf(stmp,"%s/.rcc/locks/", rcc_home_dir);
#ifdef _WIN32
    mkdir(stmp);
#else
    mkdir(stmp, 00700);
#endif

    sprintf(stmp,"%s/.rcc/locks/rcc.lock", rcc_home_dir);

    lockfd = open(stmp, O_RDWR|O_CREAT, 0644);
    if (lockfd >= 0) {
	for (err = -1, i = 0; i < (LIBRCC_LOCK_WAIT/10); i++) {
#if defined(HAVE_FLOCK)
	    err = flock(lockfd, LOCK_EX|LOCK_NB);
#elif defined(HAVE_LOCKF)
	    err = lockf(lockfd, F_TLOCK, 1);
#else
# warning "No file locking mechanism is detected"
	    err = 0; // We must believe in best
#endif
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
#if defined(HAVE_FLOCK)
			err = flock(lockfd, LOCK_EX|LOCK_NB);
#elif defined(HAVE_LOCKF)
			err = lockf(lockfd, F_TLOCK, 1);
#endif
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

#if defined(HAVE_FLOCK)
    flock(lockfd, LOCK_UN);
#elif defined(HAVE_LOCKF)
    lockf(lockfd, F_ULOCK, 1);
#endif

    close(lockfd);
    lockfd = -1;
#endif /* HAVE_SYS_FILE_H */
}
