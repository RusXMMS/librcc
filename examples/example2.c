/*
  LibRCC - translation example

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
#include <string.h>
#include <locale.h>

#include <librcc.h>


int main(int argc, char *argv[]) {
//    rcc_language_id language_id, current_language_id, english_language_id;
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

//    current_language_id = rccGetCurrentLanguage(NULL);
//    english_language_id = rccGetLanguageByName(NULL, "en");
    if (argc>1) rccSetLanguageByName(NULL, argv[1]);
    else rccSetOption(NULL, RCC_OPTION_AUTODETECT_LANGUAGE, 1);
//    language_id = rccGetCurrentLanguage(NULL);

    language = rccGetCurrentLanguageName(NULL);
    if (language) printf("Current Language: %s\n\n", language);
    else {
	printf("Unable Detect Language, using english\n\n");
	rccSetLanguageByName(NULL, "en");
    }
    
    while (fgets(buf,255,stdin)) {
	if (strlen(buf)<2) break;
	
	rccstring = rccFrom(NULL, 0, buf);
	if (!rccstring) rccstring = strdup(buf);

	if (rccstring) {
	    recoded = rccTo(NULL, 1, rccstring);
	    if (!recoded) recoded = strdup(rccstring);
	    if (recoded) {
		printf("%s", recoded);
		free(recoded);
	    } 
	    free(rccstring);
	} 
    }

    rccFree();

    return 0;
}
