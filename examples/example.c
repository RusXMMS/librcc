#include <stdio.h>
#include <locale.h>

#include <librcc.h>

static rcc_class classes[] = {
    { "input", RCC_CLASS_STANDARD, NULL, NULL, "Input Encoding" },
    { "output", RCC_CLASS_STANDARD, "LC_CTYPE", NULL, "Output Encoding" },
    { NULL }
};

main() {
    const char *language;
    char buf[255];
    char *recoded;
    int l;

    setlocale(LC_ALL, "");
    
    rccInit();
    rccInitDefaultContext(NULL, 0, 0, classes, 0);
    
    language = rccGetCurrentLanguageName(NULL);
    if (language) printf("Current Language: %s\n\n", language);
    else printf("Unable Detect Language\n\n");
    
    while (fgets(buf,255,stdin)) {
	if (strlen(buf)<2) break;
	recoded = rccRecode(NULL, 0, 1, buf, 0, NULL);
	if (recoded) {
	    printf(recoded);
	    free(recoded);
	} else printf(buf);
    }

    rccFree();
}
