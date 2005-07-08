#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "internal.h"
#include "rccconfig.h"

#define MAX_HOME_CHARS 96
#define XPATH_LANGUAGE "//Language[@name]"

static xmlDocPtr xmlctx = NULL;

static const char *rccXmlGetText(xmlNodePtr node) {
    if ((node)&&(node->children)&&(node->children->type == XML_TEXT_NODE)&&(node->children->content)) return node->children->content;
}

int rccXmlInit() {
    FILE *f;
    char config[MAX_HOME_CHARS + 32];

    xmlXPathContextPtr xpathctx;    
    xmlXPathObjectPtr obj;
    xmlNodeSetPtr node_set;
    unsigned long i, nnodes;
    xmlNodePtr cnode, pnode, node;
    xmlAttrPtr attr;
    const char *lang, *fullname;
    unsigned int pos, cpos, npos;
    
    xmlInitParser();
    xmlInitCharEncodingHandlers();
    xmlKeepBlanksDefault(0);

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

    
	// Load Extra Languages
    if (config[0]) {
	xmlctx = xmlReadFile(config, NULL, 0);
	if (!xmlctx) goto clear;
	
	xpathctx = xmlXPathNewContext(xmlctx);
	if (!xpathctx) goto clear;
	    
	obj = xmlXPathEvalExpression(XPATH_LANGUAGE, xpathctx);
	if (!obj) goto clear;
	
	node_set = obj->nodesetval;
	if (!node_set) return 0;
	
	for (pos = 0; rcc_default_languages[pos].sn; pos++);
	if (pos == RCC_MAX_LANGUAGES) goto clear;

	for (npos = 0; rcc_default_language_names[npos].sn; npos++);

	
	nnodes = node_set->nodeNr;
        for (i=0;i<nnodes;i++) {
	    pnode = node_set->nodeTab[i];
	    attr = xmlHasProp(pnode, "name");
	    lang = attr->children->content;
	    
	    if ((!lang)||(!lang[0])) continue;
	    
	    for (cpos=1,fullname=NULL,node=pnode->children;node;node=node->next) {
	    	if (node->type != XML_ELEMENT_NODE) continue;
		if (!xmlStrcmp(node->name, "Charsets")) {
		    for (cpos = 0, cnode=node->children;cnode;cnode=cnode->next) {
			if (cnode->type != XML_ELEMENT_NODE) continue;
			if ((!xmlStrcmp(cnode->name, "Charset"))&&(rccXmlGetText(cnode))&&(cpos<RCC_MAX_CHARSETS)) {
			        rcc_default_languages[pos].charsets[cpos++] = rccXmlGetText(cnode);
			}
		    }
		} else if (!xmlStrcmp(node->name, "FullName")) {
		    if (rccXmlGetText(node)) fullname = rccXmlGetText(node);
		}
	    }
	    
	    if (cpos > 1) {
		rcc_default_languages[pos].sn = lang;
		rcc_default_languages[pos].charsets[0] = "Default";
		rcc_default_languages[pos].charsets[cpos] = NULL;
		rcc_default_languages[pos].engines[0] = &rcc_default_engine;
		rcc_default_languages[pos].engines[1] = NULL;
		if ((fullname)&&(npos<RCC_MAX_LANGUAGES)) {
		    rcc_default_language_names[npos].sn = lang;
		    rcc_default_language_names[npos].name = fullname;
		    rcc_default_language_names[++npos].sn = NULL;
		    rcc_default_language_names[npos].name = NULL;
		}

		rcc_default_languages[++pos].sn = NULL;
		if (pos == RCC_MAX_LANGUAGES) break;
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
    
    obj = xmlXPathEvalExpression(req, xpathctx);
    if (obj) {
	node_set = obj->nodesetval;
	if (node_set->nodeNr > 0) {
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
    
    xmlXPathContextPtr xpathctx;    
    xmlDocPtr doc = NULL;
    xmlNodePtr pnode, lnode, onode, llnode, cnode, enode, node;
    unsigned char oflag = 0, llflag = 0, cflag;
    const char *oname;
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
    mkdir(config, 00644);
    
    sprintf(config,"%s/.rcc/%s.xml",rcc_home_dir,name);
    fd = open(config, O_CREAT|O_RDWR,00644);
    if (fd == -1) goto clear;
    flock(fd, LOCK_EX);
    
    if ((!fstat(fd, &st))&&(st.st_size)) {
	doc = xmlReadFd(fd, config, NULL, 0);
    }
    
    if (!doc) {
	doc = xmlNewDoc("1.0");
	if (!doc) goto clear;
    }

    xpathctx = xmlXPathNewContext(doc);
    if (!xpathctx) goto clear;

    pnode = rccNodeFind(xpathctx, XPATH_SELECTED);
    if (pnode) {
	lnode = rccNodeFind(xpathctx, XPATH_SELECTED_LANGUAGE);
	onode = rccNodeFind(xpathctx, XPATH_SELECTED_OPTIONS);
	llnode = rccNodeFind(xpathctx, XPATH_SELECTED_LANGS);
    } else {
	pnode = xmlNewChild((xmlNodePtr)doc, NULL, "Config", NULL);
	lnode = NULL;
	onode = NULL;
	llnode = NULL;
    }
    
    if (lnode) xmlNodeSetContent(lnode, rccGetSelectedLanguageName(ctx));
    else lnode = xmlNewChild(pnode,NULL, "Language", rccGetSelectedLanguageName(ctx));

    if (onode) oflag = 1;
    else onode = xmlNewChild(pnode, NULL, "Options", NULL);
    
    for (i=0;i<RCC_MAX_OPTIONS;i++) {
	oname = rccGetOptionName(i);
	if (!oname) continue;
	
	if (oflag) node = rccNodeFind(xpathctx, XPATH_SELECTED_OPTION, oname);
	else node = NULL;

	if (rccOptionIsDefault(ctx, (rcc_option)i)) strcpy(value, rcc_option_nonconfigured);
	else sprintf(value, "%i", rccGetOption(ctx, (rcc_option)i));

	if (node) xmlNodeSetContent(node, value);
	else {
	    node = xmlNewChild(onode, NULL, "Option", value);
	    xmlSetProp(node, "name", oname);
	}
    }

    if (llnode) llflag = 1;
    else llnode = xmlNewChild(pnode, NULL, "Languages", NULL);

    languages = ctx->languages;
    classes = ctx->classes;
    for (i=1;languages[i];i++) {
	language = languages[i];
	cfg = rccCheckConfig(ctx, (rcc_language_id)i);
	if (!cfg) continue;
	
	if (llflag) lnode = rccNodeFind(xpathctx, XPATH_SELECTED_LANG, language->sn);
	else lnode = NULL;
	
	if (lnode) {
	    enode = rccNodeFind(xpathctx, XPATH_SELECTED_ENGINE, language->sn);
	    cnode = rccNodeFind(xpathctx, XPATH_SELECTED_CLASSES, language->sn);
	} else {
	    lnode = xmlNewChild(llnode, NULL, "Language", NULL);
	    xmlSetProp(lnode, "name", language->sn);
	    enode = NULL;
	    cnode = NULL;
	}
	
	if (enode) xmlNodeSetContent(enode, rccConfigGetSelectedEngineName(cfg));
	else xmlNewChild(lnode, NULL, "Engine", rccConfigGetSelectedEngineName(cfg));
	
	if (cnode) cflag = 1;
	else {
	    cnode = xmlNewChild(lnode, NULL, "Classes", NULL);
	    cflag = 0;
	}
	
	for (j=0;classes[j];j++) {
	    cl = classes[j];
	    
	    if (cflag) node = rccNodeFind(xpathctx, XPATH_SELECTED_CLASS, language->sn, cl->name);
	    else node = NULL;
	
	    if (node) xmlNodeSetContent(node, rccConfigGetSelectedCharsetName(cfg, (rcc_class_id)j));
	    else {
		node = xmlNewChild(cnode, NULL, "Class", rccConfigGetSelectedCharsetName(cfg, (rcc_class_id)j));
		xmlSetProp(node, "name", cl->name);
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
	    fsync(fd);
	    flock(fd, LOCK_UN);
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
	flock(fd, LOCK_EX);
	if ((!fstat(fd, &st))&&(st.st_size)) {
	    doc = xmlReadFd(fd, name, NULL, 0);
	} 
	flock(fd, LOCK_UN);
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
	flock(sysfd, LOCK_EX);
	if ((!fstat(sysfd, &st))&&(st.st_size)) {
	    sysdoc = xmlReadFd(sysfd, name, NULL, 0);
	} 
	flock(sysfd, LOCK_UN);
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
	oname = rccGetOptionName((rcc_option)i);
	if (!oname) continue;

	node = rccNodeFind(xpathctx, XPATH_SELECTED_OPTION, oname);
	if (!node) node = rccNodeFind(sysxpathctx, XPATH_SELECTED_OPTION, oname);
	if (node) {
	    tmp = rccXmlGetText(node);
	    if ((tmp)&&(strcasecmp(tmp,rcc_option_nonconfigured))) err = rccSetOption(ctx, (rcc_option)i, (rcc_option_value)atoi(tmp));
	    else err = -1;
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

    if ((!ctx->current_language)&&(rccGetOption(ctx, RCC_CONFIGURED_LANGUAGES_ONLY))) {
	ctx->current_config = rccGetCurrentConfig(ctx);
    	ctx->configure = 1;
    }
    
    return 0;
}
