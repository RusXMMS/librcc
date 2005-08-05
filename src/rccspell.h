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
    rcc_speller parrents[RCC_MAX_LANGUAGE_PARRENTS+1];
};

typedef struct rcc_speller_t rcc_speller_s;

int rccSpellerGetError(rcc_speller rccspeller);


rcc_speller_result rccSpellerSized(rcc_speller speller, const char *word, size_t len, int recursion);

#endif /* _RCC_SPELL_H */
