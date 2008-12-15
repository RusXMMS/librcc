/*
  LibRCC

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as published
  by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>

#include <errno.h>

#include "config.h"

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif /* HAVE_SYS_FILE_H */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif /* HAVE_DIRENT_H */

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif /* HAVE_GETOPT_H */

#include <librcc.h>

#ifndef RCC_OPTION_TRANSLATE_SKIP_PARENT
# define RCC_OPTION_TRANSLATE_SKIP_PARENT RCC_OPTION_TRANSLATE_SKIP_PARRENT
#endif

typedef enum {
    MODE_STDIN = 0x1000,
    MODE_DIRECTORY,
    MODE_FILE,
    MODE_FILELIST
} Modes;

int mode = MODE_STDIN;

typedef enum {
    OPT_CONFIG = 'c',
    OPT_ENCODING_IN = 'e',
    OPT_FROM = 'f',
    OPT_HELP = 'h',
    OPT_LANGUAGE_IN = 'l',
    OPT_TO = 't',
    OPT_YES = 'y',
    OPT_ENCODING_OUT,
    OPT_LANGUAGE_OUT,
    OPT_TRANSLATION,
    OPT_CACHING,
    OPT_CACHE,
    OPT_AUTODETECT,
    OPT_OFFLINE,
    OPT_TIMEOUT,
    OPT_SUBDIRS,
} Options;

static struct option long_options[] = {
    {"config",			required_argument, 0, OPT_CONFIG },
    {"from", 			required_argument, 0, OPT_FROM }, 
    {"to", 			required_argument, 0, OPT_TO },
    {"force-encoding",		required_argument, 0, OPT_ENCODING_IN },
    {"force-language", 		required_argument, 0, OPT_LANGUAGE_IN },
    {"force-target-encoding",	required_argument, 0, OPT_ENCODING_OUT },
    {"force-target-language",	required_argument, 0, OPT_LANGUAGE_OUT },
    {"language-detection",	required_argument, 0, OPT_AUTODETECT },
    {"translation",		optional_argument, 0, OPT_TRANSLATION },
    {"caching",			optional_argument, 0, OPT_CACHING },
    {"cache",			required_argument, 0, OPT_CACHE },
    {"timeout",			required_argument, 0, OPT_TIMEOUT },
    {"force",			no_argument, 0, OPT_YES },
#ifdef RCC_OPTION_OFFLINE
    {"allow-offline-processing",no_argument, 0, OPT_OFFLINE },
#endif /* RCC_OPTION_OFFLINE */
    {"disable-subdirs",		no_argument, 0, OPT_SUBDIRS },
    {"stdin",			no_argument, &mode, MODE_STDIN },
    {"directory", 		no_argument, &mode, MODE_DIRECTORY },
    {"file",			no_argument, &mode, MODE_FILE },
    {"filelist",		no_argument, &mode, MODE_FILELIST },
    {"help",			no_argument, 0, OPT_HELP },
    { 0, 0, 0, 0 }
};

void Usage(int argc, char *argv[]) {
    printf(
"Usage:\n"
" %s [options] [mode] [file|directory]\n"
"  Modes:\n"
"	--stdin		- Convert stdin to stdout\n"
"	--directory	- Convert file names in specified directory\n"
"	--file		- Convert specified file\n"
"	--filelist	- Convert all files writed on stdin\n"
"	--help		- Help message\n"
"\n"
"  Options:\n"
"	-c <config>	- Specify configuration name\n"
"	-f <class>	- Source class ('in' is default)\n"
"	-t <class>	- Output class ('out' is default)\n"
"	-e <enc>	- Force specified source encoding (autodetection)\n"
"	-l <lang>	- Force specified source language (from LC_CTYPE)\n"
"	--force-target-encoding=<enc>\n"
"			- Convert to the specified encoding\n"
"	--force-target-language=<enc>\n"
"			- Translate to the specified language\n"
"	--caching=[mode]\n"
"			- Use recodings cache. Following modes are supported\n"
"			off 		- Turn off\n"
"			use		- Use cached values (default)\n"
"			add		- Add new recodings to cache\n"
"			replace		- Replace encodings in cache\n"
"	--cache=<name>\n"
"			- Use specified cache database instead of default one\n"
"	--translation=[mode]\n"
"			- Enable translation. Following modes are supported:\n"
"			full		- Full\n"
"			skip_parent	- Skip translation to parent lang\n"
"			skip_related	- Skip translation between related langs\n"
"			english		- Translate to english (default)\n"
"			transliterate	- Transliterate\n"
"	--language-detection=[mode]\n"
"			- Lanuage autodetection. Following modes are supported:\n"
"			off		- Current language is considered\n"
"			on		- Use only configured langs (default)\n"
"			all		- Try everything (slow)\n"
"	--timeout=<us>\n"
"			- Specify recoding timeout in microseconds (1s default)\n"
"\n"
"	-y		- Do not ask any question\n"
"	--disable-subdirs\n"
"			- Do not descend into the sub directories\n"
"\n"
" Language Relations:\n"
"  To prevent unneccesary translations the concept of related/parent languages is\n"
"  introduced. For each language you can specify a parent language.\n"
"  skip_parent   translation option will turn off translation to parent language\n"
"  skip_related  translation option will additionaly turn off translation from\n"
"  parent language.\n"
"\n"
"  For example, in the default configuration Russian is parent of Ukrainian, and\n"
"  English is parent of all other languages. With \"skip_parrent\" option the\n"
"  translation from Russian to Ukrainian would be turned off, but translation\n"
"  from Ukrainian to Russian would operate. With \"skip_related\" option the\n"
"  translation in both directions would be disabled\n"
"\n\n"
" Language Detection:\n"
"  Current version uses aspell dictionaries to autodetect language. Therefore,\n"
"  only languages with aspell available in the system aspell dictionaries are\n"
"  autodected. Beware, if your system contains a lot of installed languages,\n"
"  the autodection may take considerable amount of time.\n"
"\n\n",
argv[0]);
}

/*
    fs: is a standard class here, we do not need fs detecting here
*/
static rcc_class classes[] = {
    { "unicode", RCC_CLASS_TRANSLATE_CURRENT, "UTF-8", NULL, "Dummy", 0 },
    { "in", RCC_CLASS_STANDARD, NULL, NULL, "Input Encoding", 0 },
    { "out", RCC_CLASS_TRANSLATE_CURRENT, "LC_CTYPE", NULL, "Output Encoding", 0 },
    { "id3", RCC_CLASS_STANDARD, "in", NULL, "ID3 Encoding", 0 },
    { "id3v2", RCC_CLASS_STANDARD, "id3", NULL, "ID3 v.2 Encoding", 0},
    { "pl", RCC_CLASS_STANDARD, "id3", NULL, "PlayList Title Encoding", 0},
    { "plfs", RCC_CLASS_STANDARD, "pl", NULL, "PlayList File Encoding", 0 },
    { "fs", RCC_CLASS_STANDARD, "LC_CTYPE", NULL, "FileSystem Encoding", 0 },
    { "oem", RCC_CLASS_STANDARD, "in", NULL, "Zip OEM Encoding", 0 },
    { "iso", RCC_CLASS_STANDARD, "in", NULL, "Zip ISO Encoding", 0 },
    { "ftp", RCC_CLASS_STANDARD, "in", NULL, "FTP Encoding", 0 },
    { NULL }
};

rcc_class_id GetClass(const char *name) {
    int i;
    
    for (i = 1; classes[i].name; i++) {
	if ((!strcasecmp(name, classes[i].name))||(!strcasecmp(name, classes[i].fullname)))
	    return i;
    }
    return (rcc_class_id)-1;
}

static char ask = 1;
static char process_subdirs = 1;
static rcc_language_id source_language_id, target_language_id;
static rcc_class_id source_class_id = 1, target_class_id = 2;
static char *efrom = NULL, *eto = NULL;

static int translate = RCC_OPTION_TRANSLATE_OFF;


char *Translate(const char *source);
int Stdin(const char *arg);
int Directory(const char *arg);

int main(int argc, char *argv[]) {
    rcc_language_id language_id, current_language_id, english_language_id;
    
    unsigned char c;
    
    char *arg = NULL;
    
    char *config_name = NULL;
    char *cache_name = NULL;
    
    char *from = "in";
    char *to = "out";
    
    unsigned char from_forced = 0;
    unsigned char to_forced = 0;
    
    char *lfrom = NULL;
    char *lto = NULL;
    
    int cache = RCC_OPTION_LEARNING_FLAG_USE;
    
    int ldetect = 0;
    int ldetect_all = 0;
    int ldetect_force = 0;

    unsigned long timeout = 0;
    char offline = 0;
    
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "yhe:f:l:t:", long_options, &option_index)) != (unsigned char)-1) {
	switch (c) {
	    case 0:
	    break;
	    case OPT_HELP:
		Usage(argc, argv);
		exit(0);		
	    break;
	    case OPT_CONFIG:
		config_name = optarg;
	    break;
	    case OPT_CACHE:
		cache_name = optarg;
	    case OPT_FROM:
		from_forced = 1;
		from = optarg;
	    break;
	    case OPT_TO:
		to_forced = 1;
		to = optarg;
	    break;
	    case OPT_ENCODING_IN:
		efrom = optarg;
	    break;
	    case OPT_ENCODING_OUT:
		eto = optarg;
	    break;
	    case OPT_LANGUAGE_IN:
		lfrom = optarg;
/*
		Selects main language, but for translation we can switch on
		autodetection. Should do it manualy.
*/		
		if (!ldetect_force) {
		    ldetect = 0;
		    ldetect_force = 1;
		}

	    break;
	    case OPT_LANGUAGE_OUT:
		lto = optarg;
	    break;
	    case OPT_TRANSLATION:
		if (!optarg)
		    translate = RCC_OPTION_TRANSLATE_TO_ENGLISH;
		else if (!strcasecmp(optarg, "full"))
		    translate = RCC_OPTION_TRANSLATE_FULL;
		else if (!strcasecmp(optarg, "skip_parent"))
		    translate = RCC_OPTION_TRANSLATE_SKIP_PARENT;
		else if (!strcasecmp(optarg, "skip_related"))
		    translate = RCC_OPTION_TRANSLATE_SKIP_RELATED;
		else if (!strcasecmp(optarg, "english"))
		    translate = RCC_OPTION_TRANSLATE_TO_ENGLISH;
		else if (!strcasecmp(optarg, "transliterate"))
		    translate = RCC_OPTION_TRANSLATE_TRANSLITERATE;
		else if (!strcasecmp(optarg, "off"))
		    translate = RCC_OPTION_TRANSLATE_OFF;
		else {
		    fprintf(stderr, "*** Unknown translation mode: %s\n\n", optarg);
		    Usage(argc, argv);
		    exit(0);
		}
		
		if (!ldetect_force) {
		    if (!strcasecmp(optarg, "off"))
			ldetect = 0;
		    else 
			ldetect = 1;
		}
	    break;
	    case OPT_CACHING:
	    	if (!optarg)
		    cache = RCC_OPTION_LEARNING_FLAG_USE;
		else if (!strcasecmp(optarg, "off"))
		    cache = 0;
		else if (!strcasecmp(optarg, "use"))
		    cache = RCC_OPTION_LEARNING_FLAG_USE;
		else if (!strcasecmp(optarg, "add"))
		    cache = RCC_OPTION_LEARNING_FLAG_USE|RCC_OPTION_LEARNING_FLAG_LEARN;
		else if (!strcasecmp(optarg, "replace"))
		    cache = RCC_OPTION_LEARNING_FLAG_LEARN;
		else {
		    fprintf(stderr, "*** Unknown caching mode: %s\n\n", optarg);
		    Usage(argc, argv);
		    exit(0);
		}
	    break;
	    case OPT_AUTODETECT:
		ldetect_force = 1;

		if (!optarg) ldetect = 1;
		else if (!strcasecmp(optarg, "off")) {
		    ldetect = 0;
		    ldetect_force = 1;
		} else if (!strcasecmp(optarg, "on")) {
		    ldetect = 1;
		    ldetect_all = 0;
		    ldetect_force = 1;
		} else if (!strcasecmp(optarg, "all")) {
		    ldetect = 1;
		    ldetect_all = 1;
		    ldetect_force = 1;
		}
	    break;
	    case OPT_TIMEOUT:
		timeout = atoi(optarg);
	    break;
	    case OPT_OFFLINE:
		offline = 1;
	    break;
	    case OPT_SUBDIRS:
		process_subdirs = 0;
	    break;
	    case OPT_YES:
		ask = 0;
	    break;
	    default:
		Usage(argc, argv);
		exit(0);
	}
    }
    
    if (optind < argc) {
	if ((optind + 1) < argc) {
	    fprintf(stderr, "*** Invalid non-option arguments:\n");
	    for (;optind < argc;optind++) {
		puts(argv[optind]);
	    }
	    fprintf(stderr, "\n\n");
	    Usage(argc,argv);
	    exit(0);
	}
	arg = argv[optind];
    }

    switch (mode) {
	case MODE_DIRECTORY:
	    if (!from_forced) from = "fs";
	    if (!to_forced) to = "fs";
	break;
	default:
	    ;
    }
	
    setlocale(LC_ALL, "");
    


    rccInit();
    rccInitDefaultContext(NULL, 0, 0, classes, 0);
    rccInitDb4(NULL, cache_name, 0);

    if (timeout) rccSetOption(NULL, RCC_OPTION_TIMEOUT, timeout);

    if (config_name) rccLoad(NULL, config_name);


    rccSetOption(NULL, RCC_OPTION_LEARNING_MODE, cache);

    if (translate != RCC_OPTION_TRANSLATE_OFF) 
	rccSetOption(NULL, RCC_OPTION_TRANSLATE, translate);

    if (ldetect) {
	rccSetOption(NULL, RCC_OPTION_AUTODETECT_LANGUAGE, 1);
	if (ldetect_all) {
	    rccSetOption(NULL, RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, 0);
	}
    }
    
	// DS: More checks, sometimes we can skip that.
    if ((lfrom)||(lto)) {
//	if (lfrom) rccSetOption(NULL, RCC_OPTION_AUTODETECT_LANGUAGE, 1);
	rccSetOption(NULL, RCC_OPTION_CONFIGURED_LANGUAGES_ONLY, 0);
    }

#ifdef RCC_OPTION_OFFLINE
    if (offline)
	rccSetOption(NULL, RCC_OPTION_OFFLINE, 1);
#endif /* RCC_OPTION_OFFLINE */

    if (from) {
	source_class_id = GetClass(from);
	if (source_class_id == (rcc_class_id)-1) {
	    rccFree();
	    fprintf(stderr, "*** Invalid source class (%s) specified\n", from);
	    exit(1);
	}
    } 
    if (to) {
	target_class_id = GetClass(to);
	if (target_class_id == (rcc_class_id)-1) {
	    rccFree();
	    fprintf(stderr, "*** Invalid target class (%s) specified\n", to);
	    exit(1);
	}
    } 
    
    current_language_id = rccGetCurrentLanguage(NULL);
    english_language_id = rccGetLanguageByName(NULL, "en");

    if (lfrom) {
	source_language_id = rccGetLanguageByName(NULL, lfrom);
	if (source_language_id == (rcc_language_id)-1) {
	    rccFree();
	    fprintf(stderr, "*** Invalid source language (%s) specified\n", lfrom);
	    exit(1);
	}
    } else source_language_id = current_language_id;
    
    if (lto) {
	target_language_id = rccGetLanguageByName(NULL, lto);
	if (target_language_id == (rcc_language_id)-1) {
	    rccFree();
	    fprintf(stderr, "*** Invalid target language (%s) specified\n", lto);
	    exit(1);
	}
    } else target_language_id = current_language_id;
    
    if (source_language_id == target_language_id) {
	language_id = source_language_id;
	
	if (language_id != current_language_id) {
	    if ((rccSetLanguage(NULL, language_id))||(!rccGetCurrentLanguageName(NULL))) {
		rccFree();
		fprintf(stderr, "*** Unable to set the specified language (%s)\n", rccGetLanguageName(NULL, language_id));
		exit(1);
	    }
	} else {
	    // Automatic
	    if (!rccGetCurrentLanguageName(NULL)) {
		if (current_language_id != english_language_id) {
		    language_id = english_language_id;
		    rccSetLanguage(NULL, english_language_id);
		}
		
		if (!rccGetCurrentLanguageName(NULL)) {
		    rccFree();
		    fprintf(stderr, "*** Default language (%s) is not configured\n", rccGetLanguageName(NULL, current_language_id));
		    exit(1);
		}
	    }
	}
	
    } else {
	language_id = (rcc_language_id)-1;
	
	    // Checking if languages are selectable
	if ((rccSetLanguage(NULL, source_language_id))||(!rccGetCurrentLanguageName(NULL))) {
	    rccFree();
	    fprintf(stderr, "*** Unable to set source language (%s)\n", rccGetLanguageName(NULL, source_language_id));
	    exit(1);
	}
	if ((rccSetLanguage(NULL, target_language_id))||(!rccGetCurrentLanguageName(NULL))) {
	    rccFree();
	    fprintf(stderr, "*** Unable to set target language (%s)\n", rccGetLanguageName(NULL, target_language_id));
	    exit(1);
	}
    }
    
    switch (mode) {
	case MODE_STDIN:
	    Stdin(arg);
	break;
	case MODE_DIRECTORY:
	    Directory(arg);
	break;
	case MODE_FILE:
	    fprintf(stderr, "*** Mode (FILE) is not supported in current version\n");
	break;
	case MODE_FILELIST:
	    fprintf(stderr, "*** Mode (FILELIST) is not supported in current version\n");
	break;
    }

    
    rccFree();

    return 0;
}

// DS. Dynamicaly raise string length?
int Stdin(const char *arg) {
    char *res;
    char buf[16384];

    while (fgets(buf,16384,stdin)) {
	res = Translate(buf);
	fprintf(stdout, res?res:buf);
	if (res) free(res);
    }
    
    return 0;
}

char *Fullname(const char *path, const char *name) {
    char *res;
    
    res = (char*)malloc(strlen(path) + strlen(name) + 2);
    if (res) {
	if (path[strlen(path)-1] == '/')
	    sprintf(res, "%s%s",path,name);
	else
	    sprintf(res, "%s/%s",path,name);
    }
    return res;
}

// DS: We do not follow symbolic links (add option?)
// DS: Skipping everything begining with point (system files)
int Directory(const char *arg) {
    int err;
    struct stat st;
    
    DIR *dir;
    struct dirent *entry;
    char *res;
    char answer;
    
    char stmp[255];
    char *fn, *nfn;
    
    if (!arg) arg = ".";
    
    printf("Processing directory: %s\n", arg);
    
    dir = opendir(arg);
    if (!dir) {
	fprintf(stderr, "*** Failed to process directory: %s\n", arg);
	return -1;
    }
    
    entry = readdir(dir);
    while (entry) {
	if (entry->d_name[0] == '.') {
	    entry = readdir(dir);
	    continue;
	}
	
	res = Translate(entry->d_name);
	if (res) {
	    if (strcmp(res, entry->d_name)) {
		if (ask) {
		    printf("Rename \"%s\" to \"%s\" (y/[n]) ", entry->d_name, res);
		    scanf("%c", &answer);
		    if (answer != '\n') fgets(stmp, 255, stdin);
		    answer = ((answer=='y')||(answer=='Y'))?1:0;
		} else {
		    answer = 1;
		}
		
		if (answer) {
		    fn = Fullname(arg, entry->d_name);
		    nfn = Fullname(arg, res);
		    if ((fn)&&(nfn)) {
			if (!lstat(nfn, &st)) {
			    if (!ask) {
    				printf("Trying rename \"%s\" to \"%s\"\n", entry->d_name, res);
			    }
			    
			    if (S_ISDIR(st.st_mode)) {
				printf("*** Directory with that name exists, skipping\n");
				answer = 0;
			    } else {
				printf("*** File exists, overwrite (y/[n]) ");
				scanf("%c", &answer);
				if (answer != '\n') fgets(stmp, 255, stdin);
				answer = ((answer=='y')||(answer=='Y'))?1:0;
			    }
			}
			if (answer) {
			    err = rename(fn, nfn);
			}
		    } else err = ENOMEM;
		    
		    if (fn) free(fn);
		    if (nfn) free(nfn);
		    
		    if (err) {
    			printf("*** Renaming \"%s\" to \"%s\" is failed (errno: %u)\n", entry->d_name, res, errno);
		    } else if (!ask) {
    			printf("Rename completed: \"%s\" to \"%s\"\n", entry->d_name, res);
		    }
		}
	    }
	    free(res);
	}
	entry = readdir(dir);
    }
    closedir(dir);
    
    if (process_subdirs) {
	dir = opendir(arg);
	if (!dir) return 0;
	
	entry = readdir(dir);
	while (entry) {
	    if (entry->d_name[0] == '.') {
		entry = readdir(dir);
		continue;
	    }

	    fn = Fullname(arg, entry->d_name);
	    if (fn) {
		if ((!lstat(fn, &st))&&((S_ISDIR(st.st_mode)))) {
		    Directory(fn);
		}
		free(fn);
	    }
	    entry = readdir(dir);
	}
	closedir(dir);
    }
    
    
    return 0;
}

char *Translate(const char *source) {
    rcc_string rccstring;
    char *recoded, *stmp;

    if (strlen(source)<2) return NULL;

    if (source_language_id != target_language_id) {
	rccSetLanguage(NULL, source_language_id);
    }

    if (efrom) rccstring = rccFromCharset(NULL, efrom, source);
    else rccstring = rccFrom(NULL, source_class_id, source);
    
    if (!rccstring) return NULL;

    if (source_language_id != target_language_id)
	rccSetLanguage(NULL, target_language_id);

    if (eto) {
	if (translate = RCC_OPTION_TRANSLATE_OFF) {
	    stmp = rccTo(NULL, target_class_id, rccstring);
	    if (stmp) {
		recoded = rccRecodeCharsets(NULL, "UTF-8", eto, stmp);
		if (recoded)  free(stmp);
		else recoded = stmp;
	    } else recoded = NULL;
	    
	} else {
	    recoded = rccToCharset(NULL, eto, rccstring);
	}
    } else recoded = rccTo(NULL, target_class_id, rccstring);
    
    free(rccstring);
    return recoded;        
}

