# This file is part of GNOME Translate.
#
# Copyright (C) 2004 Jean-Yves Lefort.
#
# As a special exception to the GNOME Translate licensing terms,
# Jean-Yves Lefort gives unlimited permission to copy, distribute and
# modify this file.

dnl AM_PATH_ASPELL([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl
AC_DEFUN([AM_PATH_ASPELL],
[ASPELL_CFLAGS=""
ASPELL_LIBS="-laspell"

ac_save_CFLAGS="$CFLAGS"
ac_save_LIBS="$LIBS"
CFLAGS="$CFLAGS $ASPELL_CFLAGS"
LIBS="$LIBS $ASPELL_LIBS"

AC_MSG_CHECKING([for Aspell])
AC_RUN_IFELSE([
#include <aspell.h>

int main() {
  new_aspell_config;

  exit(0);
}
], [found=yes], [found=no], [found=yes])
AC_MSG_RESULT($found)

CFLAGS="$ac_save_CFLAGS"
LIBS="$ac_save_LIBS"

if test $found = yes; then
	ifelse([$1],, :, [$1])
else
	ASPELL_CFLAGS=""
	ASPELL_LIBS=""
	ifelse([$2],, :, [$2])
fi

AC_SUBST(ASPELL_CFLAGS)
AC_SUBST(ASPELL_LIBS)])
