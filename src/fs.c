/*
  LibRCC - Module providing file names recoding

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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif /* HAVE_MNTENT_H */

#include "internal.h"
#include "rcciconv.h"
#include "rccconfig.h"

#ifndef strndup
static char *rccStrndup(const char *str, size_t n) {
    char *res;
    
    n = STRNLEN(str, n);
    res = (char*)malloc((n+1)*sizeof(char));
    if (!res) return res;
    strncpy(res, str, n);
    res[n] = 0;
    return res;
}
#define strndup rccStrndup
#endif /* !strndup */

static char *rccCreateFullName(const char *path, const char *filename) {
    unsigned int i;
    char *name;
	
    if (!path) {
	if (filename) return strdup(filename);
	else return strdup("/");
    } else if (!filename) return strdup(path);
    
    
    i = strlen(path);
    name = (char*)malloc((i+strlen(filename)+2)*sizeof(char));
    if (!name) return NULL;
    
    if ((path[i-1]=='/')||(filename[0]=='/'))
	sprintf(name, "%s%s", path, filename);
    else
	sprintf(name, "%s/%s", path, filename);

    return name;
}

static int rccIsFile(const char *filename) {
    struct stat st;

#ifdef HAVE_SYS_STAT_H
# ifdef S_ISREG
    if ((!stat(filename,&st))&&(S_ISREG(st.st_mode))) return 1;
# else /* S_ISREG */
    if (!stat(filename,&st)) return 1;
# endif /* S_ISREG */
#endif /* HAVE_SYS_STAT_H */
    return 0;
}

static char *rccCheckFile(const char *prefix, const char *name) {
    char *temp;
    
    temp = rccCreateFullName(prefix, name);
    if ((!temp)||(rccIsFile(temp))) return temp;

    free(temp);
    return NULL;
}

/* Converts: 'filename' to 'prefix/name' using 'fspath' */
int rccFS0(rcc_language_config config, const char *fspath, const char *filename, char **prefix, char **name) {
#ifdef HAVE_MNTENT_H
    FILE *mtab;
    struct mntent *fsentry;
    char *lastprefix;
#endif /* HAVE_MNTENT_H */

    const char *tmp = NULL;
    size_t len;

    if (fspath) {
	len = strlen(fspath);
	if (!len) return 1;
	
	if (!strncmp(filename, fspath, len)) tmp = filename + strlen(fspath);
#ifdef HAVE_MNTENT_H
    } else {
	lastprefix = config->ctx->lastprefix;
	
	/* only required with non-english mount directories */
	len = strlen(lastprefix);
	if ((len)&&(!strncmp(filename, lastprefix, len))) {
	    tmp = filename + len;
	}
	
	if (tmp) mtab = NULL;
	else mtab = setmntent(_PATH_MNTTAB, "r");
	if (mtab) {
	    while (!feof(mtab)) {
		fsentry = getmntent(mtab);
		if ((fsentry)&&(fsentry->mnt_dir)) {
		    len = strlen(fsentry->mnt_dir);
		    if (len > 1) {
			if (!strncmp(filename, fsentry->mnt_dir, len)) {
			    tmp = filename + len;
			    if (len<RCC_MAX_PREFIX_CHARS) strcpy(lastprefix, fsentry->mnt_dir);
			    break;
			}
		    }
		}
	    }
	    endmntent(mtab);
	}
#endif /* HAVE_MNTENT_H */
    }

    if (!tmp) return 1;
    
    *name = strdup(tmp);
    *prefix = strndup(filename, (tmp-filename));

    if ((!*name)||(!*prefix)) {
	if (*name) free(*name);
	if (*prefix) free(*prefix);
	return -1;    
    }

    return 0;
}

/* Normalizes 'prefix/name' using 'fspath' 
returns:
    -1		Error
     0  	Okey
     bit 1	Exact Match
     bit 2	Memory cleanup isn't required
*/
int rccFS1(rcc_language_config config, const char *fspath, char **prefix, char **name) {
    char *result;
    char *path, *filename;
    
    path = *prefix;
    filename = *name;
    

    if ((path)&&(filename)) {
	result = rccCreateFullName(path, filename);
	if (!result) return -1;
    } else if (filename) result = filename;
    else if (path) result = path;
    else return -1;
    
    if (rccIsASCII(result)) {
	*name = result;
	if ((path)&&(filename)) return 1;
	return 3;
    }

	// Checking without recoding in case of autodetection
    if (rccGetOption(config->ctx, RCC_OPTION_AUTODETECT_FS_NAMES)) {
	if (rccIsFile(result)) {
	    *prefix = NULL;
	    *name = result;
	    
	    if ((path)&&(filename)) return 1;
	    return 3;
	}
    }
    
    if (rccFS0(config, fspath, result, prefix, name)) {
	*prefix = NULL;
	*name = result;
	
	if ((path)&&(filename)) return 0;
	return 2;
    }

    if ((path)&&(filename)) free(result);
    
    return 0;    
}

/* Checks if 'prefix/name' is accessible using 'icnv' recoding. In case of 
sucess returns pointer on statically allocated memory, and NULL overwise */
char *rccFS2(rcc_language_config config, iconv_t icnv, const char *prefix, const char *name) {
    size_t size;
    char *tmpbuffer = config->ctx->tmpbuffer;

    if (icnv) {    
	size = rccIConvInternal(config->ctx, icnv, name, 0);
	if (size == (size_t)-1) return NULL;
    } else {
	strncpy(tmpbuffer, name, RCC_MAX_STRING_CHARS);
	tmpbuffer[RCC_MAX_STRING_CHARS] = 0;
    }

    return rccCheckFile(prefix, tmpbuffer);
}

/* Tries to find 'name' encoding in 'prefix/name' file. Returns pointer on
statically allocated string with correct filename or NULL. */
char *rccFS3(rcc_language_config config, rcc_class_id class_id, const char *prefix, const char *name) {
    unsigned int i;
    char *result;
    rcc_charset charset; 
    rcc_language *language;
    iconv_t icnv = config->fsiconv;

    if ((rccGetOption(config->ctx, RCC_OPTION_AUTODETECT_FS_NAMES))&&(icnv)) {
	result = rccFS2(config, icnv, prefix, name);
	if (result) return result;
    }
    
    result = rccFS2(config, config->iconv_to[class_id], prefix, name);
    if (result) {
	if (icnv) rccIConvClose(icnv);
	config->fsiconv = NULL;
	return result;
    }

    if (rccGetOption(config->ctx, RCC_OPTION_AUTODETECT_FS_NAMES)) {
	language = config->language;
	if (language->charsets[0]) {
	    for (i=1;(!result);i++) {
		charset = language->charsets[i];
		if (!charset) break;
		
		if (icnv) rccIConvClose(icnv);
		if (rccIsUTF8(charset)) icnv = NULL;
		else {
		    icnv = rccIConvOpen(charset, "UTF-8");
		}

		result = rccFS2(config, icnv, prefix, name);
	    }
	}
    }
    if (result) config->fsiconv = icnv;
    else {
	if (icnv) rccIConvClose(icnv);
	config->fsiconv = NULL;
    }
    
    return result;
}

char *rccFS5(rcc_context ctx, rcc_language_config config, rcc_class_id class_id, const char *utfstring) {
    int err;
    char *prefix, *name;
    char *result;

    if (rccIsASCII(utfstring)) return strdup(utfstring);

    name = NULL;
    prefix = NULL;
	    
    rccMutexLock(ctx->mutex);
    rccMutexLock(config->mutex);
    err = rccFS0(config, NULL, utfstring, &prefix, &name);
    if (err>=0) {
	result = rccFS3(config, class_id, prefix, name);
	rccMutexUnLock(config->mutex);
	rccMutexUnLock(ctx->mutex);
	if (!err) {
	    if (prefix) free(prefix);
	    free(name);
	}
	return result;
    }
    rccMutexUnLock(config->mutex);
    rccMutexUnLock(ctx->mutex);

    return NULL;
}
