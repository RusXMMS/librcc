/*
  LibRCC - module responsible for application synchronization (write access to
  a shared config mainly)

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as published
  by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
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
