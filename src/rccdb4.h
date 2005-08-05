#ifndef _RCC_DB4_H
#define _RCC_DB4_H

#include "librcc.h"

#include "../config.h"

#ifdef HAVE_DB_H
# include <db.h>
#endif /* HAVE_DB_H */

struct db4_context_t {
#ifdef HAVE_DB_H
    DB_ENV *dbe;
    DB *db;
#endif /* HAVE_DB_H */
    
    rcc_db4_flags flags;
};

typedef struct db4_context_t db4_context_s;
typedef struct db4_context_t *db4_context;

db4_context rccDb4CreateContext(const char *dbpath, rcc_db4_flags flags);
void rccDb4FreeContext(db4_context ctx);

int rccDb4SetKey(db4_context ctx, const char *orig, size_t olen, const char *string);
char *rccDb4GetKey(db4_context ctx, const char *orig, size_t olen);

#endif /* _RCC_DB4_H */
