lib_LTLIBRARIES = librccui.la
librccui_la_SOURCES = librccui.c \
    rccmenu.c rccmenu.h \
    rccnames.c rccnames.h \
    internal.h \
    null.c    
librccui_la_LIBADD = ../src/librcc.la @XML_LIBS@
librccui_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@

if HAVE_GTK
lib_LTLIBRARIES += librccgtk.la
librccgtk_la_SOURCES = librccui.c \
    rccmenu.c rccmenu.h \
    rccnames.c rccnames.h \
    internal.h \
    gtk.c    
librccgtk_la_LIBADD = ../src/librcc.la @GTK1_LIBS@ @XML_LIBS@
librccgtk_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@
librccgtk_la_CPPFLAGS = -I${top_srcdir}/src @XML_INCLUDES@ @GTK1_INCLUDES@
endif

if HAVE_GTK2
lib_LTLIBRARIES += librccgtk2.la
librccgtk2_la_SOURCES = librccui.c \
    rccmenu.c rccmenu.h \
    rccnames.c rccnames.h \
    internal.h \
    gtk.c    
librccgtk2_la_LIBADD = ../src/librcc.la @GTK2_LIBS@ @XML_LIBS@
librccgtk2_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@
librccgtk2_la_CPPFLAGS = -I${top_srcdir}/src @XML_INCLUDES@ @GTK2_CFLAGS@
endif

if HAVE_GTK3
lib_LTLIBRARIES += librccgtk3.la
librccgtk3_la_SOURCES = librccui.c \
    rccmenu.c rccmenu.h \
    rccnames.c rccnames.h \
    internal.h \
    gtk.c    
librccgtk3_la_LIBADD = ../src/librcc.la @GTK3_LIBS@ @XML_LIBS@
librccgtk3_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@
librccgtk3_la_CPPFLAGS = -I${top_srcdir}/src @XML_INCLUDES@ @GTK3_CFLAGS@
endif

AM_CPPFLAGS = -I${top_srcdir}/src @XML_INCLUDES@
include_HEADERS = librccui.h
