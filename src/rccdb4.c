/*
  LibRCC - interface to BerkleyDB

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
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#include "internal.h"
#include "rcchome.h"
#include "rccdb4.h"

#define DATABASE "autolearn.db"

db4_context rccDb4CreateContext(const char *dbpath, rcc_db4_flags flags) {
    int err;
    db4_context ctx;
#ifdef HAVE_DB_H
    DB_ENV *dbe;
    DB *db;
    
# ifndef DB_LOG_AUTOREMOVE
#  ifdef DB_VERSION_MISMATCH
    char stmp[160];
#  endif /* DB_VERSION_MISMATCH */
# endif /* DB_LOG_AUTOREMOVE */
    
    err = db_env_create(&dbe, 0);
    if (err) return NULL;

# ifdef DB_LOG_AUTOREMOVE
    dbe->set_flags(dbe, DB_LOG_AUTOREMOVE, 1);
    dbe->set_lg_max(dbe, 131072);
    
    err = rccLock();
    if (!err) {
	err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_TXN|DB_USE_ENVIRON|DB_INIT_LOCK|DB_INIT_MPOOL|DB_RECOVER, 00644);
	rccUnLock();
    } 
# else /* DB_LOG_AUTOREMOVE */
    err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL, 00644);
#  ifdef DB_VERSION_MISMATCH
    if (err == DB_VERSION_MISMATCH) {
	if (!rccLock()) {    
	    err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_LOCK|DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN|DB_USE_ENVIRON|DB_PRIVATE|DB_RECOVER, 0);
	    dbe->close(dbe, 0);
	    dbe->remove(dbe, dbpath, 0);
	    rccUnLock();
	} else {
	    err = -1;
	    dbe->close(dbe, 0);
	}
	
	if (err) return NULL;

	if (strlen(dbpath)<128) {
	    sprintf(stmp, "%s/log.0000000001", dbpath);
	    remove(stmp);
	}
	    
	err = db_env_create(&dbe, 0);
	if (err) return NULL;
	    
	err = dbe->open(dbe, dbpath, DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL, 00644);
	
    }
#  endif /* DB_VERSION_MISMATCH */
# endif /* DB_LOG_AUTOREMOVE */

    if (err) {
//	fprintf(stderr, "BerkelyDB initialization failed: %i (%s)\n", err, db_strerror(err));
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

int rccDb4SetKey(db4_context ctx, const char *orig, size_t olen, const char *string) {
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

char *rccDb4GetKey(db4_context ctx, const char *orig, size_t olen) {
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
