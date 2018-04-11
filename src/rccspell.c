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

#include <stdio.h>
#include <stdlib.h>

#include "internal.h"
#include "rccspell.h"

rcc_speller rccSpellerCreate(const char *lang) {
#ifdef HAVE_ASPELL
    rcc_speller rccspeller;
    AspellSpeller *speller = NULL;
    AspellConfig *config;
    AspellCanHaveError *possible_err;

    if (!lang) return NULL;

    rccspeller = (rcc_speller)malloc(sizeof(rcc_speller_s));
    if (!rccspeller) return rccspeller;
    
    config = new_aspell_config();

    if (config) {    
	if (aspell_config_replace(config, "encoding", "utf-8")&&aspell_config_replace(config, "master", lang)) {
	    possible_err = new_aspell_speller(config);
	    if (aspell_error_number(possible_err) == 0) {
		speller = to_aspell_speller(possible_err);
	    }
	}
	delete_aspell_config(config);
    }
    
    rccspeller->speller = speller;
    rccspeller->parents[0] = NULL;
    return rccspeller;
#else 
    return NULL;
#endif /* HAVE_ASPELL */
}

void rccSpellerFree(rcc_speller rccspeller) {
#ifdef HAVE_ASPELL
    if ((rccspeller)&&(rccspeller->speller))
	delete_aspell_speller(rccspeller->speller);
    free(rccspeller);
#endif /* HAVE_ASPELL */
}

int rccSpellerGetError(rcc_speller rccspeller) {
    if ((!rccspeller)||(!rccspeller->speller)) return -1;
    return 0;
}

int rccSpellerAddParent(rcc_speller speller, rcc_speller parent) {
    unsigned int i;
    if ((!speller)||(!parent)) return -1;
    
    for (i=0;speller->parents[i];i++);
    if (i >= RCC_MAX_LANGUAGE_PARENTS) return -1;
    speller->parents[i++] = parent;
    speller->parents[i] = NULL;
    
    return 0;
}

rcc_speller_result rccSpellerSized(rcc_speller speller, const char *word, size_t len, int recursion) {
#ifdef HAVE_ASPELL
    rcc_speller_result result, saved_result = (rcc_speller_result)0;
    unsigned int i;
    int res;

    if (rccSpellerGetError(speller)) return (rcc_speller_result)RCC_SPELLER_INCORRECT;

    if (recursion) {
	for (i=0; speller->parents[i]; i++) {
	    result = rccSpellerSized(speller->parents[i], word, len, 0);
	    if ((result == RCC_SPELLER_CORRECT)||(result == RCC_SPELLER_PARENT)) return RCC_SPELLER_PARENT;
	    if ((result == RCC_SPELLER_ALMOST_CORRECT)||(result == RCC_SPELLER_ALMOST_PARENT)) saved_result = RCC_SPELLER_ALMOST_PARENT;
	}
    }
    
    if (saved_result) return saved_result;

    res = aspell_speller_check(speller->speller, word, len?len:-1);
    return res<=0?RCC_SPELLER_INCORRECT:RCC_SPELLER_CORRECT;    
#endif /* HAVE_ASPELL */
    return 0;
}

rcc_speller_result rccSpeller(rcc_speller speller, const char *word) {
    return rccSpellerSized(speller, word, 0, 1);
}

int rccSpellerResultIsOwn(rcc_speller_result res) {
    if ((res == RCC_SPELLER_ALMOST_CORRECT)||(res == RCC_SPELLER_CORRECT)) return 1;
    return 0;
}

int rccSpellerResultIsPrecise(rcc_speller_result res) {
    if ((res == RCC_SPELLER_PARENT)||(res == RCC_SPELLER_CORRECT)) return 1;
    return 0;
}

int rccSpellerResultIsCorrect(rcc_speller_result res) {
    if ((res == RCC_SPELLER_ALMOST_CORRECT)||(res == RCC_SPELLER_CORRECT)) return 1;
    if ((res == RCC_SPELLER_ALMOST_PARENT)||(res == RCC_SPELLER_PARENT)) return 1;
    return 0;
}
