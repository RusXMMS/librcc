#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#include "internal.h"
#include "rccdb4.h"

#define DATABASE "autolearn.db"

int rccInitDb4(rcc_context ctx, const char *name, rcc_db4_flags flags) {
    size_t size;
    char *dbname;
    
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    if (!name) name = "default";

    size = strlen(rcc_home_dir) + strlen(name) + 32;
    dbname = (char*)malloc(size*sizeof(char));
    if (!dbname) return -1;

    sprintf(dbname,"%s/.rcc/",rcc_home_dir);
    mkdir(dbname, 00644);
    
    sprintf(dbname,"%s/.rcc/%s.db/",rcc_home_dir,name);
    mkdir(dbname, 00644);

    ctx->db4ctx = rccDb4CreateContext(dbname, flags);
    free(dbname);	
    
    if (!ctx->db4ctx) return -1;

    return 0;
}

db4_context rccDb4CreateContext(const char *dbpath, rcc_db4_flags flags) {
    int err;
    db4_context ctx;
#ifdef HAVE_DB_H
    DB_ENV *dbe;
    DB *db;
    
    err = db_env_create(&dbe, 0);
    if (err) return NULL;
    
    err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL, 0);
    if (err) {
	dbe->close(dbe, 0);
	return NULL;
    }
    
    err = db_create(&db, dbe, 0);
    if (err) {
	dbe->close(dbe, 0);
	return NULL;
    }
    
    err = db->open(db, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0);
    if (err) {
	db->close(db, 0);
	dbe->close(dbe, 0);
	return NULL;
    } 
#endif /* HAVE_DB_H */

    ctx = (db4_context)malloc(sizeof(db4_context_s));
    if (!ctx) {
#ifdef HAVE_DB_H
    	db->close(db, 0);
	dbe->close(dbe, 0);
#endif /* HAVE_DB_H */
	return NULL;
    }
    
#ifdef HAVE_DB_H
    ctx->db = db;
    ctx->dbe = dbe;
#endif /* HAVE_DB_H */
    ctx->flags = flags;
    return ctx;
}

void rccDb4FreeContext(db4_context ctx) {
    if (ctx) {
#ifdef HAVE_DB_H
	ctx->db->close(ctx->db, 0);
	ctx->dbe->close(ctx->dbe, 0);
#endif /* HAVE_DB_H */
	free(ctx);
    }
}

#ifdef HAVE_DB_H
static void rccDb4Strip(DBT *key) {
    size_t size;
    char *str;
    
    str = (char*)key->data;
    size = key->size;
    
    while ((size > 0)&&((*str==' ')||(*str=='\n')||(*str==0))) {
	str++;
	size--;
    }
    while ((size > 0)&&((str[size-1]==' ')||(str[size-1]=='\n')||(str[size-1]==0))) {
	size--;
    }
    
    key->size = size;
    key->data = str;
}
#endif /* HAVE_DB_H */

int rccDb4SetKey(db4_context ctx, const char *orig, size_t olen, const rcc_string string) {
#ifdef HAVE_DB_H
    DBT key, data;
#endif /* HAVE_DB_H */

    if ((!ctx)||(!orig)||(!string)) return -1;
    
#ifdef HAVE_DB_H
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    
    key.data = (char*)orig;
    key.size = olen?olen:strlen(orig); /* No ending zero */
    data.data = (char*)string;
    data.size = strlen(string)+1;

    rccDb4Strip(&key);
    if (key.size < RCC_MIN_DB4_CHARS) return -1;
    
    if (!ctx->db->put(ctx->db, NULL, &key, &data, 0)) return 0;
#endif /* HAVE_DB_H */

    return 1;
}

rcc_string rccDb4GetKey(db4_context ctx, const char *orig, size_t olen) {
#ifdef HAVE_DB_H
    DBT key, data;
#endif /* HAVE_DB_H */

    if ((!ctx)||(!orig)) return NULL;

#ifdef HAVE_DB_H
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.data = (char*)orig;
    key.size = olen?olen:strlen(orig); /* No ending zero */

    data.flags = DB_DBT_REALLOC;

    rccDb4Strip(&key);
    if (key.size < RCC_MIN_DB4_CHARS) return NULL;

    if (!ctx->db->get(ctx->db, NULL, &key, &data, 0)) return data.data;
#endif /* HAVE_DB_H */

    return NULL;
}
