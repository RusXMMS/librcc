#include <stdio.h>
#include <string.h>
#include <librcd.h>

main() {
    enum russian_charsets res;
    char buf[255];
    int l;
    
    while (fgets(buf,255,stdin)) {
	if (strlen(buf)<2) break;
	
	res = get_russian_charset(buf,0);
	switch(res) {
	    case RUSSIAN_CHARSET_WIN:
		printf("CP1251: ");
	    break;
	    case RUSSIAN_CHARSET_ALT:
		printf("CP866 : ");
	    break;
	    case RUSSIAN_CHARSET_KOI:
		printf("KOI8-R: ");
	    break;
	    case RUSSIAN_CHARSET_UTF8:
		printf("UTF8  : ");
	    break;
	}
	printf("%s",buf);
	if (buf[strlen(buf)-1]!='\n') printf("\n");
    }
}
