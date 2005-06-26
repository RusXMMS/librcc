lib_LTLIBRARIES = librcc.la
librcc_la_SOURCES = librcc.c \
    rcclocale.c rcclocale.h \
    lng.c lng.h \
    lngconfig.c lngconfig.h \
    config.c config.h \
    plugin.c plugin.h \
    enca.c enca.h \
    engine.c engine.h \
    rccstring.c rccstring.h \
    xml.c xml.h \
    fs.c fs.h \
    recode.c recode.h \
    internal.h
include_HEADERS = librcc.h

INCLUDES = @XML_INCLUDES@
librcc_la_LIBADD = @XML_LIBS@
librcc_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@

