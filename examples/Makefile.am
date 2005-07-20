bin_PROGRAMS= example
example_SOURCES= example.c
example_LDADD= ../src/librcc.la

if HAVE_GTK
bin_PROGRAMS+= rcc-gtk-config
rcc_gtk_config_SOURCES = rcc-gtk-config.c
rcc_gtk_config_LDADD=../ui/librccgtk.la @GTK_LIBS@
endif

EXTRA_DIST= input-russian.txt rcc.xml rcc-example.xml mpg123-rcc.patch

test-russian: example
	cat input-russian.txt | ./example

INCLUDES = -I../src -I../ui @GTK_INCLUDES@
