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

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif /* HAVE_PWD_H */

#include "rcchome.h"

char *rcc_home_dir = NULL;

void rccHomeSet() {
    char *tmp;

#ifdef HAVE_PWD_H
    struct passwd *pw;
#endif /* HAVE_PWD_H */

    tmp = getenv ("HOME");
    if (tmp) rcc_home_dir = strdup (tmp);
#ifdef HAVE_PWD_H
    else {
	setpwent ();
	pw = getpwuid(getuid ());
	endpwent ();
	if ((pw)&&(pw->pw_dir)) rcc_home_dir = strdup (pw->pw_dir);
    }
#endif /* HAVE_PWD_H */
    if (!rcc_home_dir) rcc_home_dir = strdup("/");
}

void rccHomeFree() {
    if (rcc_home_dir) {
	free(rcc_home_dir);
	rcc_home_dir = NULL;
    }
}
