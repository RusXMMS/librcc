#include <stdio.h>
#include <stdlib.h>

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

int rccSpellerSized(rcc_speller speller, const char *word, size_t len) {
#ifdef HAVE_ASPELL
    int res;
    
    if (rccSpellerGetError(speller)) return 0;
    res = aspell_speller_check(speller->speller, word, len?len:-1);
    return res<0?0:res;    
#endif /* HAVE_ASPELL */
    return 0;
}

int rccSpeller(rcc_speller speller, const char *word) {
    return rccSpellerSized(speller, word, 0);
}
