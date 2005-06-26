#include <stdio.h>

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
    return (rcc_library_handle)dlopen(filename, RTLD_NOW);
#else
    return NULL;
#endif /* RCC_PLUGINS */
}

void rccLibraryClose(rcc_library_handle handle)
{
#ifdef RCC_PLUGINS
    dlclose(handle);
#endif /* RCC_PLUGINS */
}

void* rccLibraryFind(rcc_library_handle handle, const char *symbol)
{
#ifdef RCC_PLUGINS
    return dlsym(handle, symbol);
#else
    return NULL;
#endif /* RCC_PLUGINS */
}
