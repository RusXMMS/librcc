#ifndef _RCC_ENCA_H
#define _RCC_ENCA_H

#define RCC_ENCA_SUPPORT
#define RCC_ENCA_DYNAMIC
#define RCC_ENCA_LIB "libenca.so.0"

#ifdef HPUX
# undef RCC_ENCA_DYNAMIC
#endif

#ifdef RCC_ENCA_DYNAMIC
# define RCC_ENCA_SUPPORT
#endif

int rccEncaInit();
void rccEncaFree();

#endif /* _RCC_ENCA_H */