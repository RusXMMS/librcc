#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mntent.h>

#include "librcc.h"

static char *rccCreateFullName(const char *path, const char *filename) {
    unsigned int i;
    char *name;
	
    if (!path) {
	if (filename) return strdup(filename);
	else return strdup("/");
    } else if (!filename) return strdup(path);
    
    
    i = strlen(path);
    name = (char*)malloc(i+strlen(filename)+2)*sizeof(char));
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

int rccFS0(const char *fspath, const char *filename, char **prefix, char **name) {
    FILE *mtab;
    struct mntent *fsentry;
    char *tmp;

    if (fspath) {
	tmp = strstr(filename, fspath);
	if (tmp) tmp = filename + strlen(fspath);
    } else {
	mtab = setmntent(_PATH_MNTTAB, "r");
	if (mtab) {
	    while (!feof(mtab)) {
		fsentry = getmntent(mtab);
		if ((fsentry)&&(fsentry->mnt_dir)) {
		    tmp = strstr(filename, fsentry->mnt_dir);
		    if (tmp) tmp = filename + strlen(fsentry->mnt_dir);
		}
	    }
	    endmntent(mtab);
	}
    }

    if (!tmp) tmp = filename;

    *name = strdup(tmp);
    *prefix = strndup(filename, (tmp-filename));

    if ((!*name)||(!*prefix)) {
	if (*name) free(*name);
	if (*prefix) free(*prefix);
	return -1;    
    }

    return 0;
}

int rccFS1(rcc_context *ctx, const char *fspath, char **prefix, char **name) {
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
	if (rccIsFile(name)) {
	    if ((path)&&(filename)) *name = result;
	    else if (filename) *name = strdup(filename);
	    else *name = strdup(path);
	    return 1;
	}
    }

    err = rccFS0(fspath, result, &prefix, &name);
    if ((path)&&(filename)) free(name);
    
    return err;    
}

char *rccFS2(rcc_context *ctx, iconv_t icnv, const char *prefix, const char *name) {
    if (icnv == (iconv_t)-1) return NULL;
    if (icnv == (iconv_t)-2) {
	strcpy(ctx->tmpbuffer, name);
	ctx->tmpbuffer[len] = 0;
    } else {
	err = rccIConv(ctx, icnv, name, 0);
	if (err<=0) return NULL;
    }

    return rccCheckFile(prefix, ctx->tmpbuffer);
}

char *rccFS3(rcc_context *ctx, rcc_language_id language_id, rcc_class_id class_id, const char *prefix, const char *name) {
    rcc_charset charset; 
    rcc_language *language;
    iconv_t icnv = ctx->fsiconv;

    if ((rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES))&&(icnv != (iconv_t)-1)) {
	result = rccFS2(ctx, icnv, prefix, name);
	if (result) return result;
    }
    
    result = rccFS2(ctx, ctx->iconv_to[class_id], prefix, name);
    if (result) {
	if ((icnv != (iconv_t)-1)||(icnv != (iconv_t)-2))  iconv_close(icnv);
	ctx->fsiconv = (iconv_t)-1;
	return result;
    }

    if (rccGetOption(ctx, RCC_AUTODETECT_FS_NAMES)) {
	language = ctx->language[language_id];
	if (language->charset[0]) {
	    for (i=1;(!result);i++) {
		charset = language->charsets[i];
		if (!charset) break;
		
		if ((icnv != (iconv_t)-1)&&(icnv != (iconv_t)-2)) iconv_close(icnv);

		if (strcmp(charset, "UTF-8")&&strcmp(charset, "UTF8")) icnv = (iconv_t)-2;
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
