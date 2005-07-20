#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <librcc.h>


int main() {
    const char *language;
    char buf[255];
    char *recoded;
    
    rcc_class classes[] = {
	{ "input", RCC_CLASS_STANDARD, NULL, NULL, "Input Encoding", 0 },
	{ "output", RCC_CLASS_STANDARD, "LC_CTYPE", NULL, "Output Encoding", 0 },
	{ NULL }
    };

    setlocale(LC_ALL, "");
    
    rccInit();
    rccInitDefaultContext(NULL, 0, 0, classes, 0);

    language = rccGetCurrentLanguageName(NULL);
    if (language) printf("Current Language: %s\n\n", language);
    else printf("Unable Detect Language\n\n");
    
    while (fgets(buf,255,stdin)) {
	if (strlen(buf)<2) break;
	recoded = rccRecode(NULL, 0, 1, buf);
	if (recoded) {
	    printf(recoded);
	    free(recoded);
	} else printf(buf);
    }

    rccFree();
    return 0;
}
