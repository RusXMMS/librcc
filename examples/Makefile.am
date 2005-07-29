bin_PROGRAMS= example1 example2
example1_SOURCES= example1.c
example1_LDADD= ../src/librcc.la
example2_SOURCES= example2.c
example2_LDADD= ../src/librcc.la

if HAVE_GTK
bin_PROGRAMS+= rcc-gtk-config
rcc_gtk_config_SOURCES = rcc-gtk-config.c
rcc_gtk_config_LDADD=../ui/librccgtk.la @GTK1_LIBS@
rcc_gtk_config_CPPFLAGS = @GTK1_INCLUDES@ -I../src -I../ui
endif

if HAVE_GTK2
bin_PROGRAMS+= rcc-gtk2-config
rcc_gtk2_config_SOURCES = rcc-gtk-config.c
rcc_gtk2_config_LDADD=../ui/librccgtk2.la @GTK2_LIBS@
rcc_gtk2_config_CPPFLAGS = @GTK2_CFLAGS@ -I../src -I../ui
endif

EXTRA_DIST= input-russian.txt rcc.xml rcc-example.xml mpg123-rcc.patch

test-russian: example2
	cat input-russian.txt | ./example2 ru

AM_CPPFLAGS = -I../src -I../ui
