#include <stdio.h>

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

    ctx = (db4_context)malloc(sizeof(db4_context_s));
    if (!ctx) {
    	db->close(db, 0);
	dbe->close(dbe, 0);
	return NULL;
    }
    
    ctx->db = db;
    ctx->dbe = dbe;
    ctx->flags = flags;
    return ctx;
}

void rccDb4FreeContext(db4_context ctx) {
    if (ctx) {
	ctx->db->close(ctx->db, 0);
	ctx->dbe->close(ctx->dbe, 0);
	free(ctx);
    }
}

int rccDb4SetKey(db4_context ctx, const char *orig, size_t olen, const rcc_string string) {
    int err;
    DBT key, data;

    if ((!ctx)||(!orig)||(!string)) return -1;
    
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    
    key.data = (char*)orig;
    key.size = STRNLEN(orig, olen); /* No ending zero */
    data.data = (char*)string;
    data.size = strlen(string)+1;
    
    if (key.size < RCC_MIN_DB4_CHARS) return -1;
    
    err = ctx->db->put(ctx->db, NULL, &key, &data, 0);
    return err;
}

rcc_string rccDb4GetKey(db4_context ctx, const char *orig, size_t olen) {
    int err;
    DBT key, data;

    if ((!ctx)||(!orig)) return NULL;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.data = (char*)orig;
    key.size = STRNLEN(orig, olen); /* No ending zero */

    data.flags = DB_DBT_REALLOC;

    if (key.size < RCC_MIN_DB4_CHARS) return NULL;
    
    err = ctx->db->get(ctx->db, NULL, &key, &data, 0);
    if (err) return NULL;
    return data.data;
}
