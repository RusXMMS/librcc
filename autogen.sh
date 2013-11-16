#!/bin/sh

PROG=`basename $0`

KEYFILE=src/librcc.c


# Some OS's have multiple versions (autoconf259, etc.) and don't have an 
# autoconf binary

AUTOCONF=`which autoconf`
if test x"${AUTOCONF}" != x -a -f "${AUTOCONF}"
then
    AUTOCONF=autoconf
    LIBTOOLIZE=libtoolize
    AUTOHEADER=autoheader
else
    FINDPATH=`echo ${PATH}|sed -e 's,:, ,g'` 
    AUTOCONF=`find ${FINDPATH} -name "autoconf*"|sort -r|head -1`
    LIBTOOLIZE=`find ${FINDPATH} -name "libtoolize*"|sort -r|head -1`
    AUTOHEADER=`find /usr/bin /usr/local/bin -name "autoheader*"|sort -r|head -1`
    echo "$0: autoconf: using ${AUTOCONF}"
    echo "$0: libtoolize: using ${LIBTOOLIZE}"
    echo "$0: autoheader: using ${AUTOHEADER}"
fi

AUTOMAKE=`which automake`
if test x"${AUTOMAKE}" != x -a -f "${AUTOMAKE}"
then
    AUTOMAKE=automake
    ACLOCAL=aclocal
else
    FINDPATH=`echo ${PATH}|sed -e 's,:, ,g'` 
    AUTOMAKE=`find ${FINDPATH} -name "automake*"|sort -r|head -1`
    ACLOCAL=`find ${FINDPATH} -name "aclocal*"|sort -r|head -1`
    echo "$0: automake: using ${AUTOMAKE}"
    echo "$0: aclocal: using ${ACLOCAL}"
fi

GETTEXTIZE_FLAGS=--no-changelog
AUTOPOINT_FLAGS=
LIBTOOLIZE_FLAGS=--copy

# Some OS's require /usr/local/share/aclocal

if test ! -d /usr/local/share/aclocal
then
  ACLOCAL_FLAGS='-I m4'
else
  ACLOCAL_FLAGS='-I m4 -I /usr/local/share/aclocal'
fi
AUTOHEADER_FLAGS=-Wall
AUTOMAKE_FLAGS='--add-missing --copy -Wall'
AUTOCONF_FLAGS=-Wno-obsolete


if [ ! -f $KEYFILE ]
then
    echo "$PROG: key-file \`$KEYFILE' not found, exiting." >&2
    echo "$PROG: hint: you should run $PROG from top-level sourse directory." >&2
    exit 1
fi


boldface="`tput bold 2>/dev/null`"
normal="`tput sgr0 2>/dev/null`"

printbold() {
    echo $ECHO_N "$boldface"
    echo "$@"
    echo $ECHO_N "$normal"
}    

printerr() {
    echo "$@" >&2
}

die() {
    err=$?
    echo "$PROG: exited by previous error(s), return code was $err" >&2
    exit 1
}

compare_versions() {
    ch_min_version=$1
    ch_actual_version=$2
    ch_status=0
    IFS="${IFS=         }"; ch_save_IFS="$IFS"; IFS="."
    set $ch_actual_version
    for ch_min in $ch_min_version; do
        ch_cur=`echo $1 | sed 's/[^0-9].*$//'`; shift # remove letter suffixes
        if [ -z "$ch_min" ]; then break; fi
        if [ -z "$ch_cur" ]; then ch_status=1; break; fi
        if [ $ch_cur -gt $ch_min ]; then break; fi
        if [ $ch_cur -lt $ch_min ]; then ch_status=1; break; fi
    done
    IFS="$ch_save_IFS"
    return $ch_status
}

REQUIRED_M4MACROS=${REQUIRED_M4MACROS:-}
FORBIDDEN_M4MACROS=${FORBIDDEN_M4MACROS:-}
require_m4macro() {
    case "$REQUIRED_M4MACROS" in
	$1\ * | *\ $1\ * | *\ $1) ;;
	*) REQUIRED_M4MACROS="$REQUIRED_M4MACROS $1" ;;
    esac
}

forbid_m4macro() {
    case "$FORBIDDEN_M4MACROS" in
	$1\ * | *\ $1\ * | *\ $1) ;;
	*) FORBIDDEN_M4MACROS="$FORBIDDEN_M4MACROS $1" ;;
    esac
}

add_to_cm_macrodirs() {
    case $cm_macrodirs in
    "$1 "* | *" $1 "* | *" $1") ;;
    *) cm_macrodirs="$cm_macrodirs $1";;
    esac
}

check_m4macros() {
    # construct list of macro directories
    cm_macrodirs=`$ACLOCAL --print-ac-dir`
    # aclocal also searches a version specific dir, eg. /usr/share/aclocal-1.9
    # but it contains only Automake's own macros, so we can ignore it.

    AUTOMAKE_VERSION=`$AUTOMAKE --version | sed -e '2,$ d' -e 's/ *([^()]*)$//' -e 's/.* \(.*\)/\1/' -e 's/-p[0-9]\+//'`
    # Read the dirlist file, supported by Automake >= 1.7.
    if compare_versions 1.7 $AUTOMAKE_VERSION && [ -s $cm_macrodirs/dirlist ]; then
	cm_dirlist=`sed 's/[ 	]*#.*//;/^$/d' $cm_macrodirs/dirlist`
	if [ -n "$cm_dirlist" ] ; then
	    for cm_dir in $cm_dirlist; do
		if [ -d $cm_dir ]; then
		    add_to_cm_macrodirs $cm_dir
		fi
	    done
	fi
    fi

    # Parse $ACLOCAL_FLAGS
    set - $ACLOCAL_FLAGS
    while [ $# -gt 0 ]; do
	if [ "$1" = "-I" ]; then
	    add_to_cm_macrodirs "$2"
	    shift
	fi
	shift
    done

    cm_status=0
    if [ -n "$REQUIRED_M4MACROS" ]; then
	printbold "Checking for required M4 macros..."
	# check that each macro file is in one of the macro dirs
	for cm_macro in $REQUIRED_M4MACROS; do
	    cm_macrofound=false
	    for cm_dir in $cm_macrodirs; do
		if [ -f "$cm_dir/$cm_macro" ]; then
		    cm_macrofound=true
		    break
		fi
		# The macro dir in Cygwin environments may contain a file
		# called dirlist containing other directories to look in.
		if [ -f "$cm_dir/dirlist" ]; then
		    for cm_otherdir in `cat $cm_dir/dirlist`; do
			if [ -f "$cm_otherdir/$cm_macro" ]; then
			    cm_macrofound=true
		            break
			fi
		    done
		fi
	    done
	    if $cm_macrofound; then
		:
	    else
		printerr "  $cm_macro not found"
		cm_status=1
	    fi
	done
    fi
    if [ -n "$FORBIDDEN_M4MACROS" ]; then
	printbold "Checking for forbidden M4 macros..."
	# check that each macro file is in one of the macro dirs
	for cm_macro in $FORBIDDEN_M4MACROS; do
	    cm_macrofound=false
	    for cm_dir in $cm_macrodirs; do
		if [ -f "$cm_dir/$cm_macro" ]; then
		    cm_macrofound=true
		    break
		fi
	    done
	    if $cm_macrofound; then
		printerr "  $cm_macro found (should be cleared from macros dir)"
		cm_status=1
	    fi
	done
    fi
    if [ "$cm_status" != 0 ]; then
	printerr "***Error***: some autoconf macros required to build $PKG_NAME"
	printerr "  were not found in your aclocal path, or some forbidden"
	printerr "  macros were found.  Perhaps you need to adjust your"
	printerr "  ACLOCAL_FLAGS?"
	printerr
    fi
    return $cm_status
}


# PKG_CONFIG
require_m4macro pkg.m4
check_m4macros || die



# gettextize ${GETTEXTIZE_FLAGS}
# autopoint ${AUTOPOINT_FLAGS}
# xml-i18n-toolize       || die
# intltoolize            || die
${LIBTOOLIZE} ${LIBTOOLIZE_FLAGS} --force || die
${ACLOCAL} ${ACLOCAL_FLAGS}        || die
${AUTOHEADER} ${AUTOHEADER_FLAGS}  || die
# Seems on some systems config.h.in should be newer when various m4/* stuff generated by aclocal
if [ -f config.h.in ]; then touch config.h.in; fi
${AUTOMAKE} ${AUTOMAKE_FLAGS}      || die
${AUTOCONF} ${AUTOCONF_FLAGS}      || die
