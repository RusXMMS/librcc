lib_LTLIBRARIES = libwestern.la

libdir = $(pkgdatadir)/engines

if HAVE_RCD
lib_LTLIBRARIES += librussian.la
librussian_la_SOURCES = russian.c
librussian_la_LDFLAGS = -module -avoid-version -export-symbols-regex "rccGetInfo"
endif

libwestern_la_SOURCES = western.c
libwestern_la_LDFLAGS = -module -avoid-version -export-symbols-regex "rccGetInfo"

AM_CPPFLAGS = -I../src @RCD_INCLUDES@
