lib_LTLIBRARIES =

libdir = $(pkgdatadir)/engines

if HAVE_RCD
lib_LTLIBRARIES += librussian.la
librussian_la_SOURCES = russian.c
librussian_la_LDFLAGS = -module -avoid-version -export-symbols-regex "rccGetInfo"
endif

AM_CPPFLAGS = -I../src @RCD_INCLUDES@
