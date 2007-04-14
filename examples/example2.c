#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <librcc.h>


int main(int argc, char *argv[]) {
    rcc_language_id language_id, current_language_id, english_language_id;
    rcc_string rccstring;
    const char *language;
    char buf[255];
    char *recoded;
    
    rcc_class classes[] = {
	{ "input", RCC_CLASS_STANDARD, NULL, NULL, "Input Encoding", 0 },
	{ "output", RCC_CLASS_TRANSLATE_LOCALE, "LC_CTYPE", NULL, "Output Encoding", 0 },
	{ NULL }
    };

    setlocale(LC_ALL, "");
    
    rccInit();
    rccInitDefaultContext(NULL, 0, 0, classes, 0);
    rccInitDb4(NULL, "example", 0);
    rccSetOption(NULL, RCC_OPTION_TRANSLATE, RCC_OPTION_TRANSLATE_SKIP_PARRENT);
    rccSetOption(NULL, RCC_OPTION_AUTODETECT_LANGUAGE, 1);
//    rccSetOption(NULL, RCC_OPTION_TIMEOUT, 5000000);
//    rccSetOption(NULL, RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, 0);

    current_language_id = rccGetCurrentLanguage(NULL);
    english_language_id = rccGetLanguageByName(NULL, "en");
    if (argc>1) rccSetLanguageByName(NULL, argv[1]);
    else rccSetOption(NULL, RCC_OPTION_AUTODETECT_LANGUAGE, 1);
    language_id = rccGetCurrentLanguage(NULL);

    language = rccGetCurrentLanguageName(NULL);
    if (language) printf("Current Language: %s\n\n", language);
    else {
	printf("Unable Detect Language, using english\n\n");
	rccSetLanguageByName(NULL, "en");
    }
    
    while (fgets(buf,255,stdin)) {
	if (strlen(buf)<2) break;
	
	rccstring = rccFrom(NULL, 0, buf);
	if (rccstring) {
	    recoded = rccTo(NULL, 1, rccstring);
	    if (recoded) {
		printf(recoded);
		free(recoded);
	    } else printf("Recoding from UTF-8 is failed\n");
	    free(rccstring);
	} else printf("Recoding to UTF-8 is failed\n");
    }

    rccFree();

    return 0;
}
