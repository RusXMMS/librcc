if HAVE_GLIB2
bin_PROGRAMS= rccexternal
endif

bindir = $(pkgdatadir)/

rccexternal_SOURCES= rccexternal.c \
    rcclibtranslate.c rcclibtranslate.h \
    ../src/rccdb4.c ../src/rccdb4.h \
    ../src/rcchome.c ../src/rcchome.h \
    ../src/rcclock.c ../src/rcclock.h
    

rccexternal_LDADD= @GLIB2_LIBS@ @LIBTRANSLATE_LIBS@ @BDB_LIBS@
AM_CPPFLAGS = @GLIB2_CFLAGS@ @LIBTRANSLATE_CFLAGS@ @BDB_INCLUDES@ -I../src
