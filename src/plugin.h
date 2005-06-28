#ifndef _RCC_PLUGIN_H
#define _RCC_PLUGIN_H
#include "../config.h"


#ifdef HAVE_DLOPEN
# define RCC_PLUGINS
#endif /* HAVE_DLOPEN */

typedef void *rcc_library_handle;

rcc_library_handle rccLibraryOpen(char *filename);
void rccLibraryClose(rcc_library_handle handle);
void* rccLibraryFind(rcc_library_handle handle, const char *symbol);

#endif /* _RCC_PLUGIN_H */
