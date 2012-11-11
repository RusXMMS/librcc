/*
  LibRCC - interface to BerkleyDB

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
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#include "internal.h"
#include "rcchome.h"
#include "rccdb4.h"

#define DATABASE "autolearn.db"

db4_context rccDb4CreateContext(const char *dbpath, rcc_db4_flags flags) {
    db4_context ctx;

    if (!dbpath) return NULL;

    ctx = (db4_context)malloc(sizeof(db4_context_s));
    if (!ctx) return NULL;

    memset(ctx, 0, sizeof(db4_context_s));
    ctx->dbpath = strdup(dbpath);
    ctx->flags = flags;

    if (!ctx->dbpath) {
        free(ctx);
        return NULL;
    }

    return ctx;
}

static int rccDb4InitContext(db4_context ctx, const char *dbpath, rcc_db4_flags flags) {
#ifdef HAVE_DB_H
    int err;

# ifdef DB_VERSION_MISMATCH
    char stmp[160];
# endif /* DB_VERSION_MISMATCH */

    if (ctx->initialized) {
        if ((ctx->dbe)&&(ctx->db)) return 0;
        return -1;
    }

    err = rccLock();
    if (err) return -1;

    if (ctx->initialized) {
        if ((ctx->dbe)&&(ctx->db)) return 0;
        return -1;
    }

    ctx->initialized = 1;
    rccUnLock();

    DB_ENV *dbe;
    DB *db;
    
    err = db_env_create(&dbe, 0);
    if (err) return -1;

# if defined(DB_LOG_AUTOREMOVE)
    dbe->set_flags(dbe, DB_LOG_AUTOREMOVE, 1);
    dbe->set_lg_max(dbe, 131072);
# elif defined(DB_LOG_AUTO_REMOVE)
	// Starting from berkeleydb 4.7 API has changed
    dbe->log_set_config(dbe, DB_LOG_AUTO_REMOVE, 1);
    dbe->set_lg_max(dbe, 131072);
# endif /* DB_LOG_AUTOREMOVE */

    err = rccLock();
    if (err) {
        dbe->close(dbe, 0);
        return -1;
    }
    
    err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL|DB_THREAD, 00644);
    if (err) {
	err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_LOCK|DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN|DB_USE_ENVIRON|DB_PRIVATE|DB_RECOVER|DB_THREAD, 0);
	dbe->close(dbe, 0);

	if (err) {
	    err = db_env_create(&dbe, 0);
	    if (err) {
	        rccUnLock();
	        return -1;
	    }
	    dbe->remove(dbe, dbpath, 0);
	}

	if (strlen(dbpath)<128) {
	    sprintf(stmp, "%s/log.0000000001", dbpath);
	    remove(stmp);
	}
	    
	err = db_env_create(&dbe, 0);
	if (err) {
	    rccUnLock();
	    return -1;
	}
	    
	err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL|DB_THREAD, 00644);
    }
    rccUnLock();

    if (err) {
//	fprintf(stderr, "BerkelyDB initialization failed: %i (%s)\n", err, db_strerror(err));
	dbe->close(dbe, 0);
	return -1;
    }
    
    err = db_create(&db, dbe, 0);
    if (err) {
	dbe->close(dbe, 0);
	return -1;
    }
    

    err = db->open(db, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE|DB_THREAD, 0);
    if (err) {
	db->close(db, 0);
	dbe->close(dbe, 0);
	return -1;
    } 
#endif /* HAVE_DB_H */

#ifdef HAVE_DB_H
    ctx->db = db;
    ctx->dbe = dbe;
#endif /* HAVE_DB_H */

    return 0;
}

void rccDb4FreeContext(db4_context ctx) {
    if (ctx) {
#ifdef HAVE_DB_H
	if (ctx->db) ctx->db->close(ctx->db, 0);
	if (ctx->dbe) ctx->dbe->close(ctx->dbe, 0);
#endif /* HAVE_DB_H */
	if (ctx->dbpath) free(ctx->dbpath);
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

int rccDb4SetKey(db4_context ctx, const char *orig, size_t olen, const char *string) {
#ifdef HAVE_DB_H
    DBT key, data;

    if ((!ctx)||(!orig)||(!string)) return -1;
    if (rccDb4InitContext(ctx, ctx->dbpath, ctx->flags)) return -1;
    
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

char *rccDb4GetKey(db4_context ctx, const char *orig, size_t olen) {
#ifdef HAVE_DB_H
    DBT key, data;

    if ((!ctx)||(!orig)) return NULL;
    if (rccDb4InitContext(ctx, ctx->dbpath, ctx->flags)) return NULL;

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
