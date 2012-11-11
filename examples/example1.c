/*
  LibRCC - Simple recoding example

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

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
	    printf("%s", recoded);
	    free(recoded);
	} else printf("%s", buf);
    }

    rccFree();
    return 0;
}
