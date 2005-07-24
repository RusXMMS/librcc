lib_LTLIBRARIES = librcc.la
librcc_la_SOURCES = librcc.c \
    rcclocale.c rcclocale.h \
    lng.c lng.h \
    opt.c opt.h \
    lngconfig.c lngconfig.h \
    curconfig.c curconfig.h \
    rccconfig.c rccconfig.h \
    rcclist.c rcclist.h \
    plugin.c plugin.h \
    fake_enca.h fake_rcd.h \
    rccenca.c rccenca.h \
    rccdb4.c rccdb4.h \
    engine.c engine.h \
    rccstring.c rccstring.h \
    rccxml.c rccxml.h \
    rcciconv.c rcciconv.h \
    fs.c fs.h \
    recode.c recode.h \
    internal.h
include_HEADERS = librcc.h

AM_CPPFLAGS = -I../src -DLIBRCC_DATA_DIR=\"${pkgdatadir}\" @XML_INCLUDES@ @DLOPEN_INCLUDES@ @RCD_INCLUDES@ @ENCA_INCLUDES@ @BDB_INCLUDES@
librcc_la_LIBADD = @XML_LIBS@ @DLOPEN_LIBS@ @RCD_LIBS@ @ENCA_LIBS@ @BDB_LIBS@
librcc_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@

