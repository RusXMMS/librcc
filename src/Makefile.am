lib_LTLIBRARIES = librcc.la
librcc_la_SOURCES = librcc.c \
    rcclocale.c rcclocale.h \
    lng.c lng.h \
    lngconfig.c lngconfig.h \
    rccconfig.c rccconfig.h \
    plugin.c plugin.h \
    rccenca.c rccenca.h \
    engine.c engine.h \
    rccstring.c rccstring.h \
    xml.c xml.h \
    fs.c fs.h \
    recode.c recode.h \
    internal.h
include_HEADERS = librcc.h

INCLUDES = @XML_INCLUDES@ @RCD_INCLUDES@ @ENCA_INCLUDES@
librcc_la_LIBADD = @XML_LIBS@ @RCD_LIBS@ @ENCA_LIBS@
librcc_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@

