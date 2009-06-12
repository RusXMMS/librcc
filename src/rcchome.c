/*
  LibRCC - module detecting user's home directory

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
