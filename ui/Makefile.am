if HAVE_GTK
lib_LTLIBRARIES = librccgtk.la
librccgtk_la_SOURCES = librccui.c \
    rccnames.c rccnames.h \
    internal.h \
    gtk.c    
librccgtk_la_LIBADD = @GTK_LIBS@ ../src/librcc.la
librccgtk_la_LDFLAGS = -version-info @LIBRCC_VERSION_INFO@
endif

INCLUDES = -I../src @GTK_INCLUDES@
include_HEADERS = librccui.h
