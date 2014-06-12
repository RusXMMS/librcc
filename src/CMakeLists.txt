if (NOT WIN32)
#    SET(BUILD_SHARED_LIBS ON)
endif (NOT WIN32)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

SET(HEADERS 
    librcc.h
)

SET(INTERNAL_HEADERS
    internal.h
    curconfig.h
    engine.h
    fs.h
    lng.h
    lngconfig.h
    lngrecode.h
    opt.h
    plugin.h
    rccconfig.h
    rccdb4.h
    rccenca.h
    rccexternal.h
    rcchome.h
    rcciconv.h
    rcclist.h
    rcclocale.h
    rcclock.h
    rccmutex.h
    rccspell.h
    rccstring.h
    rcctranslate.h
    rccxml.h
    recode.h
)

SET(rcc_SRCS
    librcc.c
    curconfig.c
    engine.c
    fs.c
    lng.c
    lngconfig.c
    lngrecode.c
    opt.c
    plugin.c
    rccconfig.c
    rccdb4.c
    rccenca.c
    rccexternal.c
    rcchome.c
    rcciconv.c
    rcclist.c
    rcclocale.c
    rcclock.c
    rccmutex.c
    rccspell.c
    rccstring.c
    rcctranslate.c
    rccxml.c
    recode.c
    ${INTERNAL_HEADERS}
    ${HEADERS}
)

ADD_LIBRARY(rcc SHARED ${rcc_SRCS})
TARGET_LINK_LIBRARIES(rcc ${LIBXML2_LIBRARY} ${ICONV_LIBRARY} ${ENCA_LIBRARY} ${LIBRCD_LIBRARY} ${ZLIB_LIBRARY} ${WSOCK_LIBRARY})
