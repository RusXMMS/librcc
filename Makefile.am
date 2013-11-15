SUBDIRS = src engines external ui
DIST_SUBDIRS = ${SUBDIRS} examples
EXTRA_DIST = librcc.spec

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = librcc.pc

ACLOCAL_AMFLAGS = -I m4

@SET_MAKE@

.PHONY: examples

examples:
	${MAKE} -C examples

clean-local:
	if [ -f examples/Makefile ]; then\
	    ${MAKE} -C examples clean;\
	fi
