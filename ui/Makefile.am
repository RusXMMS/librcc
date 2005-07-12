if HAVE_GTK
lib_LTLIBRARIES = librccgtk.la
librccgtk_la_SOURCES = librccui.c \
    rccmenu.c rccmenu.h \
    rccnames.c rccnames.h \
    internal.h \
    gtk.c    
librccgtk_la_LIBADD = @GTK_LIBS@ @XML_LIBS@ ../src/librcc.la
librccgtk_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@
endif

INCLUDES = -I../src @GTK_INCLUDES@ @XML_INCLUDES@
include_HEADERS = librccui.h
