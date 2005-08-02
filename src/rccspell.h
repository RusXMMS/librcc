#ifndef _RCC_SPELL_H
#define _RCC_SPELL_H

#include "../config.h"

#ifdef HAVE_ASPELL
#include <aspell.h>
#endif /* HAVE_ASPELL */

struct rcc_speller_t {
#ifdef HAVE_ASPELL
    struct AspellSpeller *speller;
#else 
    void *speller;
#endif /* HAVE_ASPELL */
};

typedef struct rcc_speller_t *rcc_speller;
typedef struct rcc_speller_t rcc_speller_s;

rcc_speller rccSpellerCreate(const char *lang);
void rccSpellerFree(rcc_speller speller);

int rccSpellerGetError(rcc_speller rccspeller);

int rccSpellerSized(rcc_speller speller, const char *word, size_t len);
int rccSpeller(rcc_speller speller, const char *word);

#endif /* _RCC_SPELL_H */
