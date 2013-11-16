lib_LTLIBRARIES = western_engine.la

libdir = $(pkgdatadir)engines

if HAVE_RCD
lib_LTLIBRARIES += librcd_engine.la
librcd_engine_la_SOURCES = librcd.c
librcd_engine_la_LDFLAGS = -module -avoid-version -export-symbols-regex "rccGetInfo"
librcd_engine_la_LIBADD = ../src/librcc.la @RCD_LIBS@
endif

if HAVE_LIBGUESS
lib_LTLIBRARIES += libguess_engine.la
libguess_engine_la_SOURCES = libguess.c
libguess_engine_la_LDFLAGS = -module -avoid-version -export-symbols-regex "rccGetInfo"
libguess_engine_la_LIBADD = ../src/librcc.la @LIBGUESS_LIBS@
endif

western_engine_la_SOURCES = western.c
western_engine_la_LDFLAGS = -module -avoid-version -export-symbols-regex "rccGetInfo"
western_engine_la_LIBADD = ../src/librcc.la


AM_CPPFLAGS = -I${top_srcdir}/src @RCD_INCLUDES@ @LIBGUESS_INCLUDES@
