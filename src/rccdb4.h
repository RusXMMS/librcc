/*
  LibRCC - interface to BerkleyDB

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

    int initialized;
    char *dbpath;
    rcc_db4_flags flags;
};

typedef struct db4_context_t db4_context_s;
typedef struct db4_context_t *db4_context;

db4_context rccDb4CreateContext(const char *dbpath, rcc_db4_flags flags);
void rccDb4FreeContext(db4_context ctx);

int rccDb4SetKey(db4_context ctx, const char *orig, size_t olen, const char *string);
char *rccDb4GetKey(db4_context ctx, const char *orig, size_t olen);

#endif /* _RCC_DB4_H */
