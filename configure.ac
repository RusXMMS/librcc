dnl Process this file with autoconf to produce a configure script.


AC_INIT(librcc, m4_esyscmd([head -n 1 VERSION | sed -e s/CVS.*$// | tr -d \\n]))
AC_CONFIG_SRCDIR([src/librcc.c])
AC_CONFIG_HEADERS(config.h)
#AM_INIT_AUTOMAKE([subdir-objects])
XC_AUTOMAKE()

AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LIBRCC_VERSION_MAJOR=`cat VERSION | sed -e s/CVS// | cut -d . -f 1 | sed -e s/^$/0/`
LIBRCC_VERSION_MINOR=`cat VERSION | sed -e s/CVS// | cut -d . -f 2 | sed -e s/^$/0/`
LIBRCC_VERSION_SUBMINOR=`cat VERSION | sed -e s/CVS// | cut -d . -f 3 | sed -e s/^$/0/`
LIBRCC_VERSION=$LIBRCC_VERSION_MAJOR.$LIBRCC_VERSION_MINOR.$LIBRCC_VERSION_SUBMINOR
LIBRCC_VERSION_INFO=`echo $LIBRCC_VERSION | awk -F. '{ printf "%d:%d:%d", $1+$2, $3, $2 }'`

AC_SUBST(LIBRCC_VERSION)
AC_SUBST(LIBRCC_VERSION_MAJOR)
AC_SUBST(LIBRCC_VERSION_MINOR)
AC_SUBST(LIBRCC_VERSION_SUBMINOR)
AC_SUBST(LIBRCC_VERSION_INFO)

LIBRCC_CVS=`cat VERSION | sed -e s/.*CVS.*/CVS/`
if test "x$LIBRCC_CVS" = "xCVS"; then
LIBRCC_CVS=1
LIBRCC_CVS_DATE=`date +%y%m%d.%H`
else 
LIBRCC_CVS=0
LIBRCC_CVS_DATE=0
fi
AC_SUBST(LIBRCC_CVS)
AC_SUBST(LIBRCC_CVS_DATE)

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AC_PROG_MAKE_SET


AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_TYPE(ssize_t, long)


pkgdatadir=${libdir}/rcc/
AC_SUBST(pkgdatadir)

CFLAGS="$CFLAGS -Wall -Wpointer-arith"
if test ! -f /usr/include/iconv.h  -a -f /usr/local/include/iconv.h ; then
    CFLAGS="$CFLAGS -I/usr/local/include"
    LIBS="$LIBS -L/usr/local/lib"
fi

dnl Checks for programs.
AC_PATH_PROG(RM, rm, /bin/rm)
AC_PATH_PROG(MV, mv, /bin/mv)
AC_PATH_PROG(TAR, tar, /bin/tar)

dnl Fixing ugly libtool, see for details
dnl http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=523750
RM="$RM -f"

dnl Checks for header files.
AC_CHECK_HEADERS(iconv.h,, [AC_MSG_ERROR(Missing iconv header)])
AC_CHECK_HEADERS(mntent.h pwd.h sys/types.h sys/stat.h sys/file.h sys/socket.h sys/un.h sys/time.h sys/select.h sys/wait.h signal.h unistd.h fcntl.h)

AC_TRY_COMPILE([#include <langinfo.h>],
    [char *codeset = nl_langinfo (CODESET);],
    [AC_DEFINE(HAVE_CODESET,,[Define if nl_langinfo(CODESET) is available.])]
)
    
dnl Checks for libraries.

dnl ***
dnl *** LibXML2 (Mandatory)
dnl ***
AC_PATH_PROG(XML2_CONFIG, xml2-config, no)
if test $XML2_CONFIG = no; then
    AC_MSG_ERROR(LibXML2 is required)
fi
XML_LIBS="\`xml2-config --libs\`"
XML_INCLUDES="\`xml2-config --cflags\`"
AC_SUBST(XML_LIBS)
AC_SUBST(XML_INCLUDES)

dnl ***
dnl *** GTK1 (Optional)
dnl ***
AC_ARG_ENABLE(gtk1,
	      [  --disable-gtk1        disable gtk1 [default=enabled]],,
	      [enable_gtk1="yes"])

if test "x$enable_gtk1" = "xyes"; then
    AC_PATH_PROG(GTK_CONFIG, gtk-config, no)
else
    GTK_CONFIG="no"
fi

if test $GTK_CONFIG = no; then
    HAVE_GTK=no
else
    GTK1_LIBS="\`gtk-config --libs\`"
    GTK1_INCLUDES="\`gtk-config --cflags\`"
    
    saved_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS `gtk-config --cflags`"
dnl *** Preprocessor using CPPFLAGS
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS `gtk-config --cflags`"
    AC_CHECK_HEADER([gtk/gtk.h], [HAVE_GTK=yes], [HAVE_GTK=no])
    CPPFLAGS=$saved_CPPFLAGS
    CFLAGS=$saved_CFLAGS
fi

AM_CONDITIONAL(HAVE_GTK, [ test $HAVE_GTK = yes ])

if test $HAVE_GTK = no; then
    GTK1_LIBS=""
    GTK1_INCLUDES=""
fi

AC_SUBST(GTK1_LIBS)
AC_SUBST(GTK1_INCLUDES)

dnl ***
dnl *** GLIB 2.0 (Optional)
dnl ***
AC_ARG_ENABLE(glib2,
	      [  --disable-glib2        disable glib2 [default=enabled]],,
	      [enable_glib2="yes"])

if test "x$enable_glib2" = "xyes"; then
	PKG_CHECK_MODULES(GLIB2, [glib-2.0 >= 2.0.0], [HAVE_GLIB2=yes], [HAVE_GLIB2=no])
else
        HAVE_GLIB2=no
fi
AM_CONDITIONAL(HAVE_GLIB2, [ test $HAVE_GLIB2 = yes ])


dnl ***
dnl *** GTK 2.0 (Optional)
dnl ***
AC_ARG_ENABLE(gtk2,
	      [  --disable-gtk2        disable gtk2 [default=enabled]],,
	      [enable_gtk2="yes"])

if test "x$enable_gtk2" = "xyes"; then
	PKG_CHECK_MODULES(GTK2, [gtk+-2.0 >= 2.0.0], [HAVE_GTK2=yes], [HAVE_GTK2=no])
else
        HAVE_GTK2=no
fi
AM_CONDITIONAL(HAVE_GTK2, [ test $HAVE_GTK2 = yes ])


dnl ***
dnl *** GTK 3.0 (Optional)
dnl ***
AC_ARG_ENABLE(gtk3,
	      [  --disable-gtk3        disable gtk3 [default=enabled]],,
	      [enable_gtk3="yes"])

if test "x$enable_gtk3" = "xyes"; then
	PKG_CHECK_MODULES(GTK3, [gtk+-3.0 >= 3.0.0], [HAVE_GTK3=yes], [HAVE_GTK3=no])
else
        HAVE_GTK3=no
fi
AM_CONDITIONAL(HAVE_GTK3, [ test $HAVE_GTK3 = yes ])


dnl ***
dnl *** Shared libraries support
dnl ***
AC_CHECK_HEADER(dlfcn.h, [AC_CHECK_LIB(dl, dlopen, [
    AC_DEFINE(HAVE_DLOPEN,1,[Defines if dlopen is available])
    DLOPEN_LIBS="-ldl"
    DLOPEN_INCLUDES=""
    HAVE_DLOPEN=yes
],[
    DLOPEN_LIBS=""
    DLOPEN_INCLUDES=""
    
    AC_CHECK_FUNCS(dlopen, [
        HAVE_DLOPEN=yes
    ],[
        HAVE_DLOPEN=no
    ])
])])


dnl ***
dnl *** Find IConv library (External / GLibc embeded)
dnl ***
EXTRA_LIBS=""
HAVE_LIBCHARSET=no
EXTERNAL_ICONV=no

AC_ARG_ENABLE( force-system-iconv,
    [  --enable-force-system-iconv	force usage of iconv library from glibc],,
    enable_force_system_iconv="no")

if test "x$enable_force_system_iconv" != "xyes"; then
    AC_CHECK_LIB(iconv, iconv_open, [
	EXTERNAL_ICONV=yes
	EXTRA_LIBS="$EXTRA_LIBS -liconv"
    ])
    AC_CHECK_HEADER(libcharset.h, [AC_CHECK_LIB(charset, locale_charset, [
	AC_DEFINE(HAVE_LIBCHARSET,1,[Defines if libRCD is available])
	HAVE_LIBCHARSET=yes
	EXTRA_LIBS="$EXTRA_LIBS -lcharset"
    ])])
fi
AM_CONDITIONAL(HAVE_LIBCHARSET, [ test "x$HAVE_LIBCHARSET" = "xyes" ])
AC_SUBST(EXTRA_LIBS)


dnl ***
dnl *** Encoding detection engines: LibRCD, Enca, LibGUESS
dnl ***
RCD_LIBS=""
RCD_INCLUDES=""
HAVE_RCD=no
ENCA_LIBS=""
ENCA_INCLUDES=""
HAVE_ENCA=no
LIBGUESS_LIBS=""
LIBGUESS_INCLUDES=""
HAVE_LIBGUESS=no

AC_ARG_ENABLE( force-dynamic-engines,
    [  --enable-force-dynamic-engines	force usage of dynamic engines],,
    enable_force_dynamic_engines="no")

if test "x$enable_force_dynamic_engines" != "xyes"; then
    AC_CHECK_HEADER(librcd.h, [AC_CHECK_LIB(rcd, rcdGetRussianCharset, [
	AC_DEFINE(HAVE_RCD,1,[Defines if libRCD is available])
	RCD_LIBS="-lrcd"
	RCD_INCLUDES=""
	HAVE_RCD=yes
    ])])
    AC_CHECK_HEADER(enca.h, [AC_CHECK_LIB(enca, enca_analyse, [ 
	AC_DEFINE(HAVE_ENCA,1,[Defines if enca is available])
	ENCA_LIBS="-lenca"
	ENCA_INCLUDES=""
	HAVE_ENCA=yes
    ])])
fi

AC_CHECK_HEADERS([libguess/libguess.h libguess.h], [AC_CHECK_LIB(guess, guess_jp, [ 
	AC_DEFINE(HAVE_LIBGUESS,1,[Defines if libguess is available])
	LIBGUESS_LIBS="-lguess"
	LIBGUESS_INCLUDES=""
	HAVE_LIBGUESS=yes
])])


AM_CONDITIONAL(HAVE_RCD, [ test "x$HAVE_RCD" = "xyes" ])
AM_CONDITIONAL(HAVE_ENCA, [ test "x$HAVE_ENCA" = "xyes" ])
AM_CONDITIONAL(HAVE_LIBGUESS, [ test "x$HAVE_LIBGUESS" = "xyes" ])

AC_SUBST(RCD_LIBS)
AC_SUBST(RCD_INCLUDES)
AC_SUBST(ENCA_LIBS)
AC_SUBST(ENCA_INCLUDES)
AC_SUBST(LIBGUESS_LIBS)
AC_SUBST(LIBGUESS_INCLUDES)

USE_DLOPEN=no
if test "x$HAVE_DLOPEN" = "xyes"; then
    if test "x$HAVE_ENCA" = "xno"; then
	HAVE_ENCA=dynamic
	USE_DLOPEN=yes
    fi
    if test "x$HAVE_RCD" = "xno"; then
	HAVE_RCD=dynamic
	USE_DLOPEN=yes
    fi
    
    if test "x$USE_DLOPEN" = "xno"; then
	DLOPEN_LIBS=""
	DLOPEN_INCLUDES=""
    fi
fi
AC_SUBST(DLOPEN_LIBS)
AC_SUBST(DLOPEN_INCLUDES)

dnl ***
dnl *** BerkeleyDB
dnl ***
AC_ARG_ENABLE( bdb,
    [  --disable-bdb			disable support of recodings caching using BerkeleyDB],,
    enable_bdb="yes")

if test "x$enable_bdb" = "xyes"; then
    AX_PATH_BDB([4],[
	BDB_LIBS="$BDB_LDFLAGS $BDB_LIBS"
	BDB_INCLUDES="$BDB_CPPFLAGS"
	HAVE_BDB=yes
    ],[
	BDB_LIBS=""
	BDB_INCLUDES=""
	HAVE_BDB=no
    ])
else
    BDB_LIBS=""
    BDB_INCLUDES=""
    HAVE_BDB=no
fi
AC_SUBST(BDB_LIBS)
AC_SUBST(BDB_INCLUDES)

dnl ***
dnl *** LibTranslate (Optional)
dnl ***
AC_ARG_ENABLE( libtranslate,
    [  --disable-libtranslate		disable usage of libtranslate],,
    enable_libtranslate="yes")

if test "x$enable_libtranslate" = "xyes"; then
    PKG_CHECK_MODULES(LIBTRANSLATE, [libtranslate], [
	HAVE_LIBTRANSLATE=yes
    ],[ 
	HAVE_LIBTRANSLATE=no
    ])

    if test $HAVE_LIBTRANSLATE = yes; then
	AC_DEFINE(HAVE_LIBTRANSLATE,1,[Defines if libtranslate is available])
	AC_CHECK_LIB(translate, translate_session_timed_translate_text, [
	    HAVE_LIBTRANSLATE_TIMED_TRANSLATE=yes
    	    AC_DEFINE(HAVE_LIBTRANSLATE_TIMED_TRANSLATE,1,[Defines if libtranslate has time limited translate])
	],[
	    HAVE_LIBTRANSLATE_TIMED_TRANSLATE=no
	])
    else
	HAVE_LIBTRANSLATE_TIMED_TRANSLATE=no
    fi
else
    HAVE_LIBTRANSLATE=no
    HAVE_LIBTRANSLATE_TIMED_TRANSLATE=no
    LIBTRANSLATE_LIBS=""
    LIBTRANSLATE_CFLAGS=""
    AC_SUBST(LIBTRANSLATE_LIBS)
    AC_SUBST(LIBTRANSLATE_CFLAGS)
fi

dnl ***
dnl *** Aspell (Optional)
dnl ***
AM_PATH_ASPELL([
    AC_DEFINE(HAVE_ASPELL,1,[Defines if aspell is available])
    HAVE_ASPELL=yes
],[
    HAVE_ASPELL=no
])


dnl ***
dnl *** Threading library
dnl ***
AC_MSG_CHECKING([for PTHREAD_LIBS])
if test "x$PTHREAD_LIBS" = "x"; then
  PTHREAD_LIBS=error
else
  AC_MSG_RESULT([$PTHREAD_LIBS])
fi

if test "x$PTHREAD_LIBS" = "xerror"; then
AC_EGREP_CPP(yes,[
#if (defined(__FreeBSD_cc_version) && __FreeBSD_cc_version <= 500001) || defined(__OpenBSD__)
	  yes
#endif
],
	PTHREAD_CFLAGS="-D_THREAD_SAFE" 
	PTHREAD_LIBS="-pthread"
)
fi
if test "x$PTHREAD_LIBS" = "xerror"; then
	AC_CHECK_LIB(pthread, pthread_mutex_lock, PTHREAD_LIBS="-lpthread")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_LIB(pthreads, pthread_mutex_lock, PTHREAD_LIBS="-lpthreads")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_LIB(c_r, pthread_mutex_lock, PTHREAD_LIBS="-lc_r")
fi
if test "x$PTHREAD_LIBS" = xerror; then
	AC_CHECK_FUNC(pthread_mutex_lock, PTHREAD_LIBS="")
fi
if test "x$PTHREAD_LIBS" = xerror; then
	PTHREAD_LIBS=""
	PTHREAD_CFLAGS=""
else
    AC_CHECK_HEADER(pthread.h, [
	AC_DEFINE(HAVE_PTHREAD,1,[Defines if pthread is available])
	HAVE_PTHREAD=yes
    ],[
	HAVE_PTHREAD=no
	PTHREAD_LIBS=""
	PTHREAD_CFLAGS=""
    ])
fi
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

    
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_CHECK_FUNCS(strcasecmp strncasecmp strdup strnlen flock lockf nanosleep usleep fsync ftruncate)

AC_OUTPUT(src/Makefile engines/Makefile external/Makefile ui/Makefile examples/Makefile Makefile librcc.spec librcc.pc)

dnl This is a remedy of distclean in src destroying .deps directory and while distcleaning external it fails to include the dependencies
dnl Stolen from curl and slightly adapted
XC_AMEND_DISTCLEAN([external src])

dnl ***
dnl *** Data directory
dnl ***
rccdir=${pkgdatadir}
while expr "${rccdir}" : "\\\$" >/dev/null; do
    rccdir=`eval echo $rccdir`
done


echo ""
echo "Configuration:"
echo "  POSIX Threading Support:               $HAVE_PTHREAD"
echo ""
echo "  External IConv Library:                $EXTERNAL_ICONV"
echo "  LibCharset Library:                    $HAVE_LIBCHARSET"
echo ""
echo "  Dynamic Engine Loading Support:        $HAVE_DLOPEN"
echo "  Enca Charset Detection Support:        $HAVE_ENCA"
echo "  LibRCD Charset Detection Support:      $HAVE_RCD"
echo "  LibGUESS Charset Detection Support:    $HAVE_LIBGUESS"
echo ""
echo "  Multilanguage support with DB4:        $HAVE_BDB"
echo "  Language autodetection using aspell:   $HAVE_ASPELL"
echo "  Libtranslate support:                  $HAVE_LIBTRANSLATE"
echo "  Libtranslate Timed Translate:          $HAVE_LIBTRANSLATE_TIMED_TRANSLATE"
echo ""
echo "User Interfaces:"
echo "  GTK User Interface:                    $HAVE_GTK"
echo "  GTK2 User Interface:                   $HAVE_GTK2"
echo "  GTK3 User Interface:                   $HAVE_GTK3"
echo ""
echo "Directories:"
echo "  RCC Data Directory:                    ${rccdir}"
echo ""
echo ""
echo ""
