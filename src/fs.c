#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mntent.h>

#include "internal.h"
#include "rcciconv.h"

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

    stat(filename,&st);
    if (S_ISREG(st.st_mode)) return 1;
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
int rccFS0(rcc_context ctx, const char *fspath, const char *filename, char **prefix, char **name) {
    FILE *mtab;
    struct mntent *fsentry;
    const char *tmp = NULL;
    size_t len;

    if (fspath) {
	len = strlen(fspath);
	if (!len) return 1;
	
	if (!strncmp(filename, fspath, len)) tmp = filename + strlen(fspath);
    } else {
	/* only required with non-english mount directories */
	len = strlen(ctx->lastprefix);
	if ((len)&&(!strncmp(filename, ctx->lastprefix, len))) {
	    tmp = filename + len;
	}
	
	if (tmp) mtab = NULL;
	else mtab = setmntent(_PATH_MNTTAB, "r");
	if (mtab) {
	    while (!feof(mtab)) {
		fsentry = getmntent(mtab);
		if ((fsentry)&&(fsentry->mnt_dir)) {
		    len = strlen(fsentry->mnt_dir);
		    if (len) {
			if (!strncmp(filename, fsentry->mnt_dir, len)) {
			    tmp = filename + len;
			    if (len<RCC_MAX_PREFIX_CHARS) strcpy(ctx->lastprefix, fsentry->mnt_dir);
			    break;
			}
		    }
		}
	    }
	    endmntent(mtab);
	}
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
int rccFS1(rcc_context ctx, const char *fspath, char **prefix, char **name) {
    int err;
    int prefix_size;
    char *result, *tmp;
    char *path, *filename;
    
    path = *prefix;
    filename = *name;
    

    if ((path)&&(filename)) {
	result = rccCreateFullName(path, filename);
	if (!result) return -1;
    } else if (filename) result = filename;
    else if (path) result = path;
    else return -1;
    
    
	// Checking without recoding in case of autodetection
    if (rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES)) {
	if (rccIsFile(result)) {
	    *prefix = NULL;
	    *name = result;
	    
	    if ((path)&&(filename)) return 1;
	    return 3;
	}
    }
    
    puts("--");
    if (rccFS0(ctx, fspath, result, prefix, name)) {
	*prefix = NULL;
	*name = result;
	
	if ((path)&&(filename)) return 0;
	return 2;
    }
    puts("++");

    if ((path)&&(filename)) free(result);
    
    return 0;    
}

/* Checks if 'prefix/name' is accessible using 'icnv' recoding. In case of 
sucess returns pointer on statically allocated memory, and NULL overwise */
const char *rccFS2(rcc_context ctx, iconv_t icnv, const char *prefix, const char *name) {
    int err;
    
    if (icnv == (iconv_t)-1) return NULL;
    if (icnv == (iconv_t)-2) {
	puts("-1");
	strncpy(ctx->tmpbuffer, name, RCC_MAX_STRING_CHARS);
	ctx->tmpbuffer[RCC_MAX_STRING_CHARS] = 0;
    } else {
	puts("-2");
	err = rccIConv(ctx, icnv, name, 0);
	if (err<=0) return NULL;
    }
    puts("ok");
    return rccCheckFile(prefix, ctx->tmpbuffer);
}

/* Tries to find 'name' encoding in 'prefix/name' file. Returns pointer on
statically allocated string with correct filename or NULL. */
const char *rccFS3(rcc_context ctx, rcc_language_id language_id, rcc_class_id class_id, const char *prefix, const char *name) {
    unsigned int i;
    const char *result;
    rcc_charset charset; 
    rcc_language *language;
    iconv_t icnv = ctx->fsiconv;

    if ((rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES))&&(icnv != (iconv_t)-1)) {
	result = rccFS2(ctx, icnv, prefix, name);
	if (result) return result;
    }
    
    result = rccFS2(ctx, ctx->iconv_to[class_id], prefix, name);
    if (result) {
	if ((icnv != (iconv_t)-1)&&(icnv != (iconv_t)-2))  iconv_close(icnv);
	ctx->fsiconv = (iconv_t)-1;
	return result;
    }

    if (rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES)) {
	language = ctx->languages[language_id];
	if (language->charsets[0]) {
	    for (i=1;(!result);i++) {
		charset = language->charsets[i];
		if (!charset) break;
		
		if ((icnv != (iconv_t)-1)&&(icnv != (iconv_t)-2)) iconv_close(icnv);

		if ((!strcmp(charset, "UTF-8"))||(!strcmp(charset, "UTF8"))) icnv = (iconv_t)-2;
		else icnv = iconv_open(charset, "UTF-8");
		
		result = rccFS2(ctx, icnv, prefix, name);
	    }
	}
    }
    if (result) ctx->fsiconv = icnv;
    else {
	if ((icnv != (iconv_t)-1)&&(icnv != (iconv_t)-2)) iconv_close(icnv);
	ctx->fsiconv = (iconv_t)-1;
    }
    
    return result;
}
