/*
  LibRCC - plugin's abstraction

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
#include <string.h>

#include "internal.h"
#include "rcchome.h"
#include "plugin.h"

#ifdef RCC_PLUGINS
# include <dlfcn.h>
# ifndef RTLD_NOW
#  define RTLD_NOW 0
# endif
#endif /* RCC_PLUGINS */


rcc_library_handle rccLibraryOpen(char *filename)
{
#ifdef RCC_PLUGINS
    if (filename) return (rcc_library_handle)dlopen(filename, RTLD_NOW);
#endif /* RCC_PLUGINS */

    return NULL;
}

void rccLibraryClose(rcc_library_handle handle)
{
#ifdef RCC_PLUGINS
    if (handle) dlclose(handle);
#endif /* RCC_PLUGINS */
}

void* rccLibraryFind(rcc_library_handle handle, const char *symbol)
{
#ifdef RCC_PLUGINS
    if ((handle)||(symbol)) return dlsym(handle, symbol);
#endif /* RCC_PLUGINS */

    return NULL;
}


static rcc_plugin_handle_s rcc_engine_handles[RCC_MAX_PLUGINS];

int rccPluginInit() {
    unsigned int i;
    
    for (i=0;i<RCC_MAX_PLUGINS;i++)
	rcc_engine_handles[i].sn = NULL;
    
    return 0;
}

void rccPluginFree() {
    unsigned int i;
    
    for (i=0;i<RCC_MAX_PLUGINS;i++)
	if (rcc_engine_handles[i].sn) {
	    rccLibraryClose(rcc_engine_handles[i].handle);
	    free(rcc_engine_handles[i].sn);
	    rcc_engine_handles[i].sn = NULL;
	}
}


rcc_plugin_handle rccPluginGetHandleByName(rcc_plugin_type type, const char *name) {
    unsigned int i;

    rcc_plugin_handle handles;
    
    if (!name) return NULL;
    
    switch(type) {
	case RCC_PLUGIN_TYPE_ENGINE:
	    handles = rcc_engine_handles;
	break;
	default:
	    return NULL;
    }
    
    for (i=0;i<RCC_MAX_PLUGINS;i++)
	if ((handles[i].sn)&&(!strcasecmp(handles[i].sn, name))) return handles+i;
    
    return NULL;
}

rcc_plugin_handle rccPluginGetFreeHandle(rcc_plugin_type type) {
    unsigned int i;

    rcc_plugin_handle handles;
    
    switch(type) {
	case RCC_PLUGIN_TYPE_ENGINE:
	    handles = rcc_engine_handles;
	break;
	default:
	    return NULL;
    }
    
    for (i=0;i<RCC_MAX_PLUGINS;i++)
	if (!handles[i].sn) return handles+i;
    
    return NULL;
}

rcc_plugin_handle rccPluginLoad(rcc_plugin_type type, const char *name) {
#ifdef RCC_PLUGINS
    void *res;
    void *infofunc;
    char *pluginfn;

    rcc_plugin_handle plugin;
    
    if (!name) return NULL;
    
    plugin = rccPluginGetHandleByName(type, name);
    if (plugin) return plugin;
    
    plugin = rccPluginGetFreeHandle(type);
    if (!plugin) return plugin;
    
    switch (type) {
	case RCC_PLUGIN_TYPE_ENGINE:
	    pluginfn = (char*)malloc((48 + strlen(rcc_home_dir) + strlen(name))*sizeof(char));
	    if (!pluginfn) return NULL;
	
	    sprintf(pluginfn, "%s/.rcc/engines/%s_engine.so", rcc_home_dir, name);
	    res = rccLibraryOpen(pluginfn);
	    if (!res) {
		sprintf(pluginfn, LIBRCC_DATA_DIR "/engines/%s_engine.so", name);
		res = rccLibraryOpen(pluginfn);
	    }
	    free(pluginfn);
	    
	    if (res) {
		infofunc = rccLibraryFind(res, "rccGetInfo");
		if (infofunc) {
		    plugin->sn = strdup(name);
		    if (plugin->sn) {
			plugin->handle = (rcc_library_handle)res;
			plugin->info_function = infofunc;
			return plugin;
		    } else rccLibraryClose(res);
		} else rccLibraryClose(res);
	    }
	break;
	default:
	    return NULL;
    }
#endif /* RCC_PLUGINS */

    return NULL;
}


rcc_engine *rccPluginEngineGetInfo(const char *name, const char *language) {
    rcc_plugin_handle handle;
    rcc_plugin_engine_info_function infofunc;

    handle = rccPluginLoad(RCC_PLUGIN_TYPE_ENGINE, name);
    if (!handle) return NULL;
    
    infofunc = (rcc_plugin_engine_info_function)(handle->info_function);
    if (!infofunc) return NULL;
    
    return infofunc(language);
}
