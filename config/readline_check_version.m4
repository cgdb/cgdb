AC_DEFUN([BASH_CHECK_LIB_TERMCAP],
[
if test "X$bash_cv_termcap_lib" = "X"; then
_bash_needmsg=yes
else
AC_MSG_CHECKING(which library has the termcap functions)
_bash_needmsg=
fi
AC_CACHE_VAL(bash_cv_termcap_lib,
[AC_CHECK_LIB(]$curses_lib_name[, tgetent, bash_cv_termcap_lib=lib]$curses_lib_name[,
  [AC_CHECK_LIB(tinfo, tgetent, bash_cv_termcap_lib=libtinfo,
    [AC_CHECK_FUNC(tgetent, bash_cv_termcap_lib=libc,
      bash_cv_termcap_lib=gnutermcap)])])])
if test "X$_bash_needmsg" = "Xyes"; then
AC_MSG_CHECKING(which library has the termcap functions)
fi
AC_MSG_RESULT(using $bash_cv_termcap_lib)
if test $bash_cv_termcap_lib = gnutermcap && test -z "$prefer_curses"; then
LDFLAGS="$LDFLAGS -L./lib/termcap"
TERMCAP_LIB="./lib/termcap/libtermcap.a"
TERMCAP_DEP="./lib/termcap/libtermcap.a"
elif test $bash_cv_termcap_lib = libtinfo; then
TERMCAP_LIB=-ltinfo
TERMCAP_DEP=
elif test $bash_cv_termcap_lib = lib$curses_lib_name; then
TERMCAP_LIB=-l$curses_lib_name
TERMCAP_DEP=
elif test $bash_cv_termcap_lib = libc; then
TERMCAP_LIB=
TERMCAP_DEP=
fi
])

dnl need: prefix exec_prefix libdir includedir CC TERMCAP_LIB
dnl require:
dnl	AC_PROG_CC
dnl	BASH_CHECK_LIB_TERMCAP

AC_DEFUN([RL_LIB_READLINE_VERSION],
[
AC_REQUIRE([BASH_CHECK_LIB_TERMCAP])

AC_MSG_CHECKING([version of installed readline library])

# What a pain in the ass this is.

# save cpp and ld options
_save_CFLAGS="$CFLAGS"
_save_LDFLAGS="$LDFLAGS"
_save_LIBS="$LIBS"

# Don't set ac_cv_rl_prefix if the caller has already assigned a value.  This
# allows the caller to do something like $_rl_prefix=$withval if the user
# specifies --with-installed-readline=PREFIX as an argument to configure

if test -z "$ac_cv_rl_prefix"; then
test "x$prefix" = xNONE && ac_cv_rl_prefix=$ac_default_prefix || ac_cv_rl_prefix=${prefix}
fi

eval ac_cv_rl_includedir=${ac_cv_rl_prefix}/include
eval ac_cv_rl_libdir=${ac_cv_rl_prefix}/lib

LIBS="$LIBS -lreadline ${TERMCAP_LIB}"
CFLAGS="$CFLAGS -I${ac_cv_rl_includedir}"
LDFLAGS="$LDFLAGS -L${ac_cv_rl_libdir}"

AC_CACHE_VAL(ac_cv_rl_version,
[AC_TRY_RUN([
#include <stdio.h>
#include <readline/readline.h>

main()
{
	FILE *fp;
	fp = fopen("conftest.rlv", "w");
	if (fp == 0) exit(1);
	fprintf(fp, "%s\n", rl_library_version ? rl_library_version : "0.0");
	fclose(fp);
	exit(0);
}
],
ac_cv_rl_version=`cat conftest.rlv`,
ac_cv_rl_version='0.0',
ac_cv_rl_version='4.2')])

CFLAGS="$_save_CFLAGS"
LDFLAGS="$_save_LDFLAGS"
LIBS="$_save_LIBS"

RL_MAJOR=0
RL_MINOR=0

# (
case "$ac_cv_rl_version" in
2*|3*|4*|5*|6*|7*|8*|9*)
	RL_MAJOR=`echo $ac_cv_rl_version | sed 's:\..*$::'`
	RL_MINOR=`echo $ac_cv_rl_version | sed -e 's:^.*\.::' -e 's:[[a-zA-Z]]*$::'`
	;;
esac

# (((
case $RL_MAJOR in
[[0-9][0-9]])	_RL_MAJOR=$RL_MAJOR ;;
[[0-9]])	_RL_MAJOR=0$RL_MAJOR ;;
*)		_RL_MAJOR=00 ;;
esac

# (((
case $RL_MINOR in
[[0-9][0-9]])	_RL_MINOR=$RL_MINOR ;;
[[0-9]])	_RL_MINOR=0$RL_MINOR ;;
*)		_RL_MINOR=00 ;;
esac

RL_VERSION="0x${_RL_MAJOR}${_RL_MINOR}"

# Readline versions greater than 4.2 have these defines in readline.h

if test "$ac_cv_rl_version" = '0.0' ; then
	AC_MSG_WARN([Could not test version of installed readline library.])
elif test $RL_MAJOR -gt 4 || { test $RL_MAJOR = 4 && test $RL_MINOR -gt 2 ; } ; then
	# set these for use by the caller
	RL_PREFIX=$ac_cv_rl_prefix
	RL_LIBDIR=$ac_cv_rl_libdir
	RL_INCLUDEDIR=$ac_cv_rl_includedir
	AC_MSG_RESULT($ac_cv_rl_version)
else

AC_DEFINE_UNQUOTED(RL_READLINE_VERSION, $RL_VERSION, [encoded version of the installed readline library])
AC_DEFINE_UNQUOTED(RL_VERSION_MAJOR, $RL_MAJOR, [major version of installed readline library])
AC_DEFINE_UNQUOTED(RL_VERSION_MINOR, $RL_MINOR, [minor version of installed readline library])

AC_SUBST(RL_VERSION)
AC_SUBST(RL_MAJOR)
AC_SUBST(RL_MINOR)

# set these for use by the caller
RL_PREFIX=$ac_cv_rl_prefix
RL_LIBDIR=$ac_cv_rl_libdir
RL_INCLUDEDIR=$ac_cv_rl_includedir

AC_MSG_RESULT($ac_cv_rl_version)

fi
])

dnl This checks to see where the include files for readline are located.
dnl It also defines the readline and history libraries
AC_DEFUN([RL_LIB_READLINE_INCLUDES],
[
AC_DEFINE(HAVE_LIBREADLINE,1, [readline library available])

AC_CHECK_HEADERS([readline.h],is_readline_header_found="true",is_readline_header_found="false")
if test "x$is_readline_header_found" = "xfalse"; then
  AC_CHECK_HEADERS([readline/readline.h],is_readline_header_found="true",is_readline_header_found="false")
  if test "x$is_readline_header_found" = "xfalse"; then
    AC_MSG_ERROR([cgdb needs readline.h or readline/readline.h to build])
  fi
fi

AC_CHECK_HEADERS([history.h],is_history_header_found="true",is_history_header_found="false")
if test "x$is_history_header_found" = "xfalse"; then
  AC_CHECK_HEADERS([readline/history.h],is_history_header_found="true",is_readline_header_found="false")
  if test "x$is_history_header_found" = "xfalse"; then
    AC_MSG_ERROR([cgdb needs history.h or readline/history.h to build])
  fi
fi
])

