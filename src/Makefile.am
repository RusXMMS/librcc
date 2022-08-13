lib_LTLIBRARIES = librcc.la
librcc_la_SOURCES = librcc.c \
    rcchome.c rcchome.h \
    rcclock.c rcclock.h \
    rcclocale.c rcclocale.h \
    lng.c lng.h \
    opt.c opt.h \
    lngconfig.c lngconfig.h \
    curconfig.c curconfig.h \
    rccconfig.c rccconfig.h \
    rcclist.c rcclist.h \
    rccmutex.c rccmutex.h \
    plugin.c plugin.h \
    rccexternal.c rccexternal.h \
    fake_enca.h fake_rcd.h \
    rccenca.c rccenca.h \
    rccdb4.c rccdb4.h \
    rccspell.c rccspell.h \
    engine.c engine.h \
    rccstring.c rccstring.h \
    rccxml.c rccxml.h \
    rcciconv.c rcciconv.h \
    rcctranslate.c rcctranslate.h \
    fs.c fs.h \
    recode.c recode.h \
    lngrecode.c lngrecode.h \
    internal.h
include_HEADERS = librcc.h

AM_CPPFLAGS = -I../src -DLIBRCC_DATA_DIR=\"${pkgdatadir}\" @XML2_CFLAGS@ @DLOPEN_INCLUDES@ @RCD_INCLUDES@ @ENCA_INCLUDES@ @BDB_INCLUDES@ @ASPELL_CFLAGS@ @PTHREAD_CFLAGS@
librcc_la_LIBADD = @XML2_LIBS@ @DLOPEN_LIBS@ @RCD_LIBS@ @ENCA_LIBS@ @BDB_LIBS@ @ASPELL_LIBS@ @PTHREAD_LIBS@ @EXTRA_LIBS@
librcc_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@
