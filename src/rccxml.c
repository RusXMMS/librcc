/*
  LibRCC - module handling XML configuration

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
#include <stdarg.h>

#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */

#include "../config.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif /* HAVE_SYS_FILE_H */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "internal.h"
#include "rccconfig.h"
#include "plugin.h"
#include "rcchome.h"

#define MAX_HOME_CHARS 96
#define XPATH_LANGUAGE "//Language[@name]"

static xmlDocPtr xmlctx = NULL;

rcc_config rccGetConfiguration() {
    return (rcc_config)xmlctx;
}

static const char *rccXmlGetText(xmlNodePtr node) {
    if ((node)&&(node->children)&&(node->children->type == XML_TEXT_NODE)&&(node->children->content)) return (const char*)node->children->content;
    return NULL;
}

int rccXmlInit(int LoadConfiguration) {
    FILE *f;
    char config[MAX_HOME_CHARS + 32];

    xmlXPathContextPtr xpathctx = NULL;
    xmlXPathObjectPtr obj = NULL;
    xmlNodeSetPtr node_set;
    unsigned long i, nnodes;
    xmlNodePtr enode, cnode, pnode, node;
    xmlAttrPtr attr;
    const char *lang, *engine_name;
    unsigned int pos, lpos, epos, cpos;
    const char *alias, *parent;
    unsigned int j, apos, rpos;
    
    rcc_engine *engine;
    
    xmlInitParser();
    xmlInitCharEncodingHandlers();
    xmlKeepBlanksDefault(0);

    if (LoadConfiguration) {
	if (strlen(rcc_home_dir)>MAX_HOME_CHARS) config[0] = 0;
	else {
	    sprintf(config, "%s/.rcc/rcc.xml", rcc_home_dir);
	    f = fopen(config, "r");
	    if (f) fclose(f);
	    else config[0] = 0;
	}
	if (!config[0]) {
	    strcpy(config, "/etc/rcc.xml");
	    f = fopen(config, "r");
    	    if (f) fclose(f);
	    else config[0] = 0;
	}
    } else config[0] = 0;

    
    for (apos=0;rcc_default_aliases[apos].alias;apos++);
    
	// Load Extra Languages
    if (config[0]) {
	xmlctx = xmlReadFile(config, NULL, 0);
	if (!xmlctx) goto clear;
	
	xpathctx = xmlXPathNewContext(xmlctx);
	if (!xpathctx) goto clear;
	    
	obj = xmlXPathEvalExpression((xmlChar*)XPATH_LANGUAGE, xpathctx);
	if (!obj) goto clear;
	
	node_set = obj->nodesetval;
	if (!node_set) goto clear;
	
	for (lpos = 0; rcc_default_languages[lpos].sn; lpos++);

	nnodes = node_set->nodeNr;
        for (i=0;i<nnodes;i++) {
	    pnode = node_set->nodeTab[i];
	    attr = xmlHasProp(pnode, (xmlChar*)"name");
	    lang = (const char*)attr->children->content;
	    
	    if ((!lang)||(!lang[0])) continue;
	    
	    pos = rccDefaultGetLanguageByName(lang);
	    if (!pos) continue;
	    if (pos == (rcc_language_id)-1) {
		for (rpos=0;rcc_default_relations[rpos].lang;rpos++);
		if (rpos < RCC_MAX_RELATIONS) {
			rcc_default_relations[rpos].parent = rcc_english_language_sn;
			rcc_default_relations[rpos++].lang = lang;
			rcc_default_relations[rpos].parent = NULL;
			rcc_default_relations[rpos].lang = NULL;
		}
		
		pos = lpos;
	    }
	    else if (pos == RCC_MAX_LANGUAGES) continue; 
	    
	    for (epos = 1, cpos = 1,node=pnode->children;node;node=node->next) {
	    	if (node->type != XML_ELEMENT_NODE) continue;
		if (!xmlStrcmp(node->name, (xmlChar*)"Charsets")) {
		    for (cnode=node->children;cnode;cnode=cnode->next) {
			if (cnode->type != XML_ELEMENT_NODE) continue;
			if ((!xmlStrcmp(cnode->name, (xmlChar*)"Charset"))&&(rccXmlGetText(cnode))&&(cpos<RCC_MAX_CHARSETS)) {
			        rcc_default_languages[pos].charsets[cpos++] = rccXmlGetText(cnode);
			}
		    }
		} 
		else if (!xmlStrcmp(node->name, (xmlChar*)"Engines")) {
		    for (enode=node->children;enode;enode=enode->next) {
			if (enode->type != XML_ELEMENT_NODE) continue;
			if ((!xmlStrcmp(enode->name, (xmlChar*)"Engine"))&&(epos<RCC_MAX_ENGINES)) {
				engine_name = rccXmlGetText(enode);
				if (!engine_name) continue;
				engine = rccPluginEngineGetInfo(engine_name, lang);
				if (!engine) continue;
				 
				rcc_default_languages[pos].engines[epos++] = engine;
			}
		    }
		}
		else if (!xmlStrcmp(node->name, (xmlChar*)"Aliases")) {
		    for (enode=node->children;enode;enode=enode->next) {
			if (enode->type != XML_ELEMENT_NODE) continue;
			if ((!xmlStrcmp(enode->name, (xmlChar*)"Alias"))&&(apos<RCC_MAX_ALIASES)) {
			    alias = rccXmlGetText(enode);
			    if (!alias) continue;
			    for (j=0;j<apos;j++)
				if (!strcasecmp(alias, rcc_default_aliases[j].alias)) break;
			    if (j<apos) {
				rcc_default_aliases[j].lang = lang;
			    } else {
				rcc_default_aliases[apos].alias = alias;
				rcc_default_aliases[apos++].lang = lang;
				rcc_default_aliases[apos].alias = NULL;
				rcc_default_aliases[apos].lang = NULL;
			    }
			}
		    }
		}
		else if (!xmlStrcmp(node->name, (xmlChar*)"Relations")) {
		    rpos = rccDefaultDropLanguageRelations(lang);
		    for (enode=node->children;enode;enode=enode->next) {
			if (enode->type != XML_ELEMENT_NODE) continue;
			if ((!xmlStrcmp(enode->name, (xmlChar*)"Parrent"))&&(rpos<RCC_MAX_RELATIONS)) {
			    parent = rccXmlGetText(enode);
			    if (!parent) continue;
			    rcc_default_relations[rpos].parent = parent;
			    rcc_default_relations[rpos++].lang = lang;
			    rcc_default_relations[rpos].parent = NULL;
			    rcc_default_relations[rpos].lang = NULL;
			}
		    }
		}
	    }
	    
	    if ((cpos > 1)||(epos > 1)) {
		rcc_default_languages[pos].sn = lang;
	        rcc_default_languages[pos].charsets[0] = rcc_default_charset;
		if (cpos > 1) rcc_default_languages[pos].charsets[cpos] = NULL;
		else {
		    rcc_default_languages[pos].charsets[1] = rcc_utf8_charset;
		    rcc_default_languages[pos].charsets[2] = NULL;
		}
		rcc_default_languages[pos].engines[0] = &rcc_default_engine;
		rcc_default_languages[pos].engines[epos] = NULL;
	    
		if (pos == lpos) rcc_default_languages[++lpos].sn = NULL;
	    }
	}
	
clear:
	if (xmlctx) {
	    if (xpathctx) {
		xmlXPathFreeContext(xpathctx);
		if (obj) {
		    xmlXPathFreeObject(obj);
		}
	    }
	}
    }

    return 0;
}

void rccXmlFree() {
    if (xmlctx) {
	xmlFreeDoc(xmlctx);
	xmlctx = NULL;
    }

    xmlCleanupCharEncodingHandlers();
    xmlCleanupParser();
}


static xmlNodePtr rccNodeFind(xmlXPathContextPtr xpathctx, const char *request, ...) {
    xmlXPathObjectPtr obj;
    xmlNodeSetPtr node_set;
    xmlNodePtr res = NULL;

    unsigned int i, args = 0;
    unsigned int size = 64;
    va_list ap;
    char *req;
    
    if (!xpathctx) return NULL;

    for (req = strstr(request, "%s"); req; req = strstr(req + 1, "%s")) args++;
    
    if (args) {
	va_start(ap, request);
	for (i=0;i<args;i++) {
	    req = va_arg(ap, char*);
	    size += strlen(req);
	}
	va_end(ap);
	
	req = (char*)malloc(size*sizeof(char));
	if (!req) return NULL;
	
	va_start(ap, request);
	vsprintf(req,request,ap);
	va_end(ap);
    } else req = (char*)request;
    
    obj = xmlXPathEvalExpression((xmlChar*)req, xpathctx);
    if (obj) {
	node_set = obj->nodesetval;
	if ((node_set)&&(node_set->nodeNr > 0)) {
	    res = node_set->nodeTab[0];
	}
	xmlXPathFreeObject(obj);
    }

    if (args) free(req);

    return res;
}

#define XPATH_SELECTED "/Config"
#define XPATH_SELECTED_LANGUAGE "/Config/Language"
#define XPATH_SELECTED_OPTIONS "/Config/Options"
#define XPATH_SELECTED_OPTION "/Config/Options/Option[@name=\"%s\"]"

#define XPATH_SELECTED_LANGS "/Config/Languages"
#define XPATH_SELECTED_LANG "/Config/Languages/Language[@name=\"%s\"]"
#define XPATH_SELECTED_ENGINE "/Config/Languages/Language[@name=\"%s\"]/Engine"
#define XPATH_SELECTED_CLASSES "/Config/Languages/Language[@name=\"%s\"]/Classes"
#define XPATH_SELECTED_CLASS "/Config/Languages/Language[@name=\"%s\"]/Classes/Class[@name=\"%s\"]"

int rccSave(rcc_context ctx, const char *name) {
    int fd;
    char *config;
    struct stat st;
    
    unsigned int i, j, size;

    rcc_language_ptr *languages;
    rcc_language_ptr language;
    rcc_language_config cfg;
    rcc_class_ptr *classes;
    rcc_class_ptr cl;
    
    xmlXPathContextPtr xpathctx = NULL;    
    xmlDocPtr doc = NULL;
    xmlNodePtr pnode, lnode, onode, llnode, cnode, enode, node;
    unsigned char oflag = 0, llflag = 0, cflag;
    rcc_option_description *odesc;
    rcc_option_value ovalue;
    const char *oname, *ovname;
    char value[16];

    int memsize;
    xmlChar *mem;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    if ((!name)||(!strcmp(name, "rcc"))||(strlen(rcc_home_dir)<3)) name = "default";

    size = strlen(rcc_home_dir) + strlen(name) + 32;
    config = (char*)malloc(size*sizeof(char));
    if (!config) return -1;

    sprintf(config,"%s/.rcc/",rcc_home_dir);
    mkdir(config, 00755);
    
    sprintf(config,"%s/.rcc/%s.xml",rcc_home_dir,name);
    fd = open(config, O_CREAT|O_RDWR,00644);
    if (fd == -1) goto clear;

#if defined(HAVE_FLOCK)
    flock(fd, LOCK_EX);
#elif defined(HAVE_LOCKF)
    lockf(fd, F_LOCK, 1);
#endif
    
    if ((!fstat(fd, &st))&&(st.st_size)) {
	doc = xmlReadFd(fd, config, NULL, 0);
    }
    
    if (!doc) {
	doc = xmlNewDoc((xmlChar*)"1.0");
	if (!doc) goto clear;
	pnode = NULL;
    } else {
	xpathctx = xmlXPathNewContext(doc);
	pnode = rccNodeFind(xpathctx, XPATH_SELECTED);
    }

    if (pnode) {
	lnode = rccNodeFind(xpathctx, XPATH_SELECTED_LANGUAGE);
	onode = rccNodeFind(xpathctx, XPATH_SELECTED_OPTIONS);
	llnode = rccNodeFind(xpathctx, XPATH_SELECTED_LANGS);
    } else {
	pnode = xmlNewChild((xmlNodePtr)doc, NULL, (xmlChar*)"Config", NULL);
	lnode = NULL;
	onode = NULL;
	llnode = NULL;
    }
    
    if (lnode) xmlNodeSetContent(lnode, (xmlChar*)rccGetSelectedLanguageName(ctx));
    else lnode = xmlNewChild(pnode,NULL, (xmlChar*)"Language", (xmlChar*)rccGetSelectedLanguageName(ctx));

    if (onode) oflag = 1;
    else onode = xmlNewChild(pnode, NULL, (xmlChar*)"Options", NULL);
    
    for (i=0;i<RCC_MAX_OPTIONS;i++) {
	odesc = rccGetOptionDescription(i);
	if (!odesc) continue;

	oname = rccOptionDescriptionGetName(odesc);
	if (!oname) continue;

	
	if (oflag) node = rccNodeFind(xpathctx, XPATH_SELECTED_OPTION, oname);
	else node = NULL;

	if (rccOptionIsDefault(ctx, (rcc_option)i)) strcpy(value, rcc_option_nonconfigured);
	else {
	    ovalue = rccGetOption(ctx, (rcc_option)i);
	    ovname = rccOptionDescriptionGetValueName(odesc, ovalue);
	    if (ovname) strcpy(value, ovname);
	    else sprintf(value, "%i", ovalue);
	}
	
	if (node) xmlNodeSetContent(node, (xmlChar*)value);
	else {
	    node = xmlNewChild(onode, NULL, (xmlChar*)"Option", (xmlChar*)value);
	    xmlSetProp(node, (xmlChar*)"name", (xmlChar*)oname);
	}
    }

    if (llnode) llflag = 1;
    else llnode = xmlNewChild(pnode, NULL, (xmlChar*)"Languages", NULL);

    languages = ctx->languages;
    classes = ctx->classes;
    for (i=1;languages[i];i++) {
	language = languages[i];
	cfg = rccCheckConfig(ctx, (rcc_language_id)i);
	if ((!cfg)||(!cfg->configured)) continue;
	
	if (llflag) lnode = rccNodeFind(xpathctx, XPATH_SELECTED_LANG, language->sn);
	else lnode = NULL;
	
	if (lnode) {
	    enode = rccNodeFind(xpathctx, XPATH_SELECTED_ENGINE, language->sn);
	    cnode = rccNodeFind(xpathctx, XPATH_SELECTED_CLASSES, language->sn);
	} else {
	    lnode = xmlNewChild(llnode, NULL, (xmlChar*)"Language", NULL);
	    xmlSetProp(lnode, (xmlChar*)"name", (xmlChar*)language->sn);
	    enode = NULL;
	    cnode = NULL;
	}
	
	if (enode) xmlNodeSetContent(enode, (xmlChar*)rccConfigGetSelectedEngineName(cfg));
	else xmlNewChild(lnode, NULL, (xmlChar*)"Engine", (xmlChar*)rccConfigGetSelectedEngineName(cfg));
	
	if (cnode) cflag = 1;
	else {
	    cnode = xmlNewChild(lnode, NULL, (xmlChar*)"Classes", NULL);
	    cflag = 0;
	}
	
	for (j=0;classes[j];j++) {
	    cl = classes[j];
	    if (cl->flags&RCC_CLASS_FLAG_SKIP_SAVELOAD) continue;
	    
	    if (cflag) node = rccNodeFind(xpathctx, XPATH_SELECTED_CLASS, language->sn, cl->name);
	    else node = NULL;
	
	    if (node) xmlNodeSetContent(node, (xmlChar*)rccConfigGetSelectedCharsetName(cfg, (rcc_class_id)j));
	    else {
		node = xmlNewChild(cnode, NULL, (xmlChar*)"Class", (xmlChar*)rccConfigGetSelectedCharsetName(cfg, (rcc_class_id)j));
		xmlSetProp(node, (xmlChar*)"name", (xmlChar*)cl->name);
	    }
	}
    }

    xmlDocDumpFormatMemory(doc,&mem,&memsize,1);
    ftruncate(fd, 0);
    lseek(fd, SEEK_SET, 0);
    if (mem) {
	write(fd, mem, memsize);
	free(mem);
    }
    
clear:
    if (config) {    
	if (fd != -1) {
	    if (doc) {
		if (xpathctx) {
		    xmlXPathFreeContext(xpathctx);
		}
		xmlFreeDoc(doc);
	    }
	    
#ifdef HAVE_FSYNC
	    fsync(fd);
#endif /* HAVE_FSYNC */

#if defined(HAVE_FLOCK)
    	    flock(fd, LOCK_UN);
#elif defined(HAVE_LOCKF)
	    lockf(fd, F_ULOCK, 1);
#endif

	    close(fd);
	}
	free(config);
    }
    
    return 0;
}

int rccLoad(rcc_context ctx, const char *name) {
    int err;
    
    int fd, sysfd;
    char *config;
    struct stat st;
    
    unsigned int i, j, size;
    const char *tmp;

    rcc_option_description *odesc;
    rcc_option_value ovalue;
    const char *oname;

    rcc_language_config cfg;
    rcc_language_ptr *languages;
    rcc_language_ptr language;
    rcc_class_ptr *classes;
    rcc_class_ptr cl;
    
    xmlXPathContextPtr xpathctx = NULL, sysxpathctx = NULL, curxpathctx;    
    xmlDocPtr doc = NULL, sysdoc = NULL;
    xmlNodePtr node, lnode;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }

    if ((!name)||(!strcmp(name, "rcc"))||(strlen(rcc_home_dir)<3)) name = "default";

    size = strlen(rcc_home_dir) + strlen(name) + 32;
    config = (char*)malloc(size*sizeof(char));
    if (!config) return -1;

    sprintf(config,"%s/.rcc/%s.xml",rcc_home_dir,name);
    fd = open(config, O_RDONLY);

    sprintf(config, "/etc/rcc/%s.xml",name);
    sysfd = open(config, O_RDONLY);
    
    free(config);

    if (fd != -1) {
#if defined(HAVE_FLOCK)
	flock(fd, LOCK_EX);
#elif defined(HAVE_LOCKF)
	lockf(fd, F_LOCK, 1);
#endif

	if ((!fstat(fd, &st))&&(st.st_size)) {
	    doc = xmlReadFd(fd, name, NULL, 0);
	} 

#if defined(HAVE_FLOCK)
	flock(fd, LOCK_UN);
#elif defined(HAVE_LOCKF)
	lockf(fd, F_ULOCK, 1);
#endif

	close(fd);
    
	if (doc) {
	    xpathctx = xmlXPathNewContext(doc);
	    if (!xpathctx) {
		xmlFreeDoc(doc);
		doc = NULL;
	    }
	}
    }

    if (sysfd != -1) {
#if defined(HAVE_FLOCK)
	flock(sysfd, LOCK_EX);
#elif defined(HAVE_LOCKF)
	lockf(sysfd, F_LOCK, 1);
#endif

	if ((!fstat(sysfd, &st))&&(st.st_size)) {
	    sysdoc = xmlReadFd(sysfd, name, NULL, 0);
	} 

#if defined(HAVE_FLOCK)
	flock(sysfd, LOCK_UN);
#elif defined(HAVE_LOCKF)
	lockf(sysfd, F_ULOCK, 1);
#endif

	close(sysfd);
    
	if (sysdoc) {
	    sysxpathctx = xmlXPathNewContext(sysdoc);
	    if (!sysxpathctx) {
		xmlFreeDoc(sysdoc);
		sysdoc = NULL;
	    }
	}
    }
    
    if ((!doc)&&(!sysdoc)) goto clear;

    node = rccNodeFind(xpathctx, XPATH_SELECTED_LANGUAGE);
    if (!node) node = rccNodeFind(sysxpathctx, XPATH_SELECTED_LANGUAGE);
    if (node) {
	tmp = rccXmlGetText(node);
	if (tmp) err = rccSetLanguageByName(ctx, tmp);
	else err = -1;
    } else err = -1;
    if (err) rccSetLanguage(ctx, 0);

    for (i=0;i<RCC_MAX_OPTIONS;i++) {
	odesc = rccGetOptionDescription(i);
	if (!odesc) continue;

	oname = rccOptionDescriptionGetName(odesc);
	if (!oname) continue;

	node = rccNodeFind(xpathctx, XPATH_SELECTED_OPTION, oname);
	if (!node) node = rccNodeFind(sysxpathctx, XPATH_SELECTED_OPTION, oname);
	if (node) {
	    tmp = rccXmlGetText(node);
	    if ((tmp)&&(strcasecmp(tmp,rcc_option_nonconfigured))) {
		ovalue = rccOptionDescriptionGetValueByName(odesc, tmp);
		if (ovalue == (rcc_option_value)-1) ovalue = (rcc_option_value)atoi(tmp);
		 err = rccSetOption(ctx, (rcc_option)i, ovalue);
	    } else err = -1;
	} else err = -1;
	if (err) rccOptionSetDefault(ctx, (rcc_option)i);
    }


    languages = ctx->languages;
    classes = ctx->classes;
    for (i=1;languages[i];i++) {
	language = languages[i];

	lnode = rccNodeFind(xpathctx, XPATH_SELECTED_LANG, language->sn);
	if (lnode) curxpathctx = xpathctx;
	else {
	    lnode = rccNodeFind(sysxpathctx, XPATH_SELECTED_LANG, language->sn);
	    if (lnode) curxpathctx = sysxpathctx;
	    else continue;
	}

	cfg = rccGetConfig(ctx, (rcc_language_id)i);
	if (!cfg) continue;
	
	node = rccNodeFind(curxpathctx, XPATH_SELECTED_ENGINE, language->sn);
	if (node) {
	    tmp = rccXmlGetText(node);
	    if (tmp) err = rccConfigSetEngineByName(cfg, tmp);
	    else err = -1;
	} else err = -1;
	if (err) rccConfigSetEngineByName(cfg, NULL);
	
	for (j=0;classes[j];j++) {
	    cl = classes[j];
	    if (cl->flags&RCC_CLASS_FLAG_SKIP_SAVELOAD) continue;
	    
	    node = rccNodeFind(curxpathctx, XPATH_SELECTED_CLASS, language->sn, cl->name);
	    if (node) {
		tmp = rccXmlGetText(node);
		if (tmp) err = rccConfigSetCharsetByName(cfg, (rcc_class_id)j, tmp);
		else err = -1;
	    } else err = -1;
	    if (err) rccConfigSetCharset(cfg, (rcc_class_id)j, 0);
	}
    }

clear:

    if (sysdoc) {
	if (sysxpathctx) {
	    xmlXPathFreeContext(sysxpathctx);
	}
	xmlFreeDoc(sysdoc);
    }
    if (doc) {
	if (xpathctx) {
	    xmlXPathFreeContext(xpathctx);
	}
	xmlFreeDoc(doc);
    }

    if ((!ctx->current_language)&&(rccGetOption(ctx, RCC_OPTION_CONFIGURED_LANGUAGES_ONLY))) {
	ctx->current_config = rccGetCurrentConfig(ctx);
    	ctx->configure = 1;
    }
    
    return 0;
}
