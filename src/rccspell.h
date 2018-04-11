/*
  LibRCC - interface to spelling libraries used by language recognition code

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

#ifndef _RCC_SPELL_H
#define _RCC_SPELL_H

#include "../config.h"

#ifdef HAVE_ASPELL
#include <aspell.h>
#endif /* HAVE_ASPELL */

#include "internal.h"

struct rcc_speller_t {
#ifdef HAVE_ASPELL
    struct AspellSpeller *speller;
#else 
    void *speller;
#endif /* HAVE_ASPELL */
    rcc_speller parents[RCC_MAX_LANGUAGE_PARENTS+1];
};

typedef struct rcc_speller_t rcc_speller_s;

int rccSpellerGetError(rcc_speller rccspeller);


rcc_speller_result rccSpellerSized(rcc_speller speller, const char *word, size_t len, int recursion);

#endif /* _RCC_SPELL_H */
