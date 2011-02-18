#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([naev], [0.5.-1], [bobbens@gmail.com])
AC_CONFIG_SRCDIR([src/naev.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build])

AM_INIT_AUTOMAKE([foreign])

# M4 defines __unix__ to be the empty string on UNIX-like systems.  This
# causes AC_DEFINE to fail if it is used to define __unix__.  This seems to
# fix the problem.
ifdef([__unix__], define([__unix__], [[__unix__]]))

#
# Utility functions
#

# If A='$B' and B='hello', then "expand_variable A" will cause A='hello'
expand_variable()
{
  while true; do
    eval local value=\$$1
    eval "$1=\"$value\""
    echo "$value" | grep -qE '(^|[^\\])\$' || break
  done
}

# Temporarily changes the value of a variable, until it is restored later.
# $1: name of variable
# $2: temporary value
define([VAR_SAVE], [
  OLD_$1="$$1"
  $1=$2
])

# Restores the saved variable.
# $1: name of variable
define([VAR_RESTORE], [
  $1="$OLD_$1"
])

#
# Checks for variables
#

# Fix missing prefix
AS_IF([test "$prefix" = "NONE"], [
  prefix="$ac_default_prefix"
])
AS_IF([test "$exec_prefix" = "NONE"], [
  exec_prefix="$prefix"
])

# Check if we have custom CFLAGS
AS_IF([test -n "$CFLAGS"], [have_custom_cflags=yes], [have_custom_cflags=no])

#
# Checks for programs.
#
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

AC_CHECK_PROG([CUT], [cut], [cut])
AS_IF([test -z "$CUT"],[
  AC_ERROR([cut not found])
])

AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen])
AC_CHECK_PROG([LUADOC], [luadoc], [luadoc])
AC_CHECK_PROG([GIT], [git], [git])

#
# Checks for build environment
#
AC_MSG_CHECKING([whether we are building utils])
AS_IF([test -d "$srcdir/utils"], [
  have_utils=yes
], [
  have_utils=no
])
AC_MSG_RESULT([$have_utils])

AC_MSG_CHECKING([whether we have docs])
AS_IF([test -d "$srcdir/docs"], [
  have_docs=yes
], [
  have_docs=no
])
AC_MSG_RESULT([$have_docs])

#
# Platform checks
#
AC_CANONICAL_HOST

AC_MSG_CHECKING([[if we need a __unix__ define]])
AC_COMPILE_IFELSE([
  #ifndef __unix__
  # if defined __APPLE__ /* add other known platforms here */
  #   error should have __unix__ but don't
  # endif
  #endif
], [AC_MSG_RESULT([no])],
[
  AC_MSG_RESULT([yes])
  AC_DEFINE([__unix__], 1,
[Define to 1 if running on a UNIX-like system where __unix__ is missing])
])

#
# Checks for command-line arguments
#

# --enable-debug
AC_MSG_CHECKING([whether to enable debug mode])
AC_ARG_ENABLE([debug],
  AC_HELP_STRING([--enable-debug=@<:@no|yes|paranoid@:>@],
     [enable debug mode (default: yes)]), [
    case "$enableval" in
      no)
        enable_debug=no
        ;;
      paranoid)
        enable_debug=paranoid
        ;;
      *)
        enable_debug=yes
        ;;
    esac
  ], [
    enable_debug=yes
  ])
AC_MSG_RESULT([$enable_debug])

AS_IF([test "$enable_debug" != "no"], [
  AS_IF([test "$have_custom_cflags" = "no"], [
    # Undo the default CFLAGS set by autoconf
    AS_IF([test "$ac_cv_prog_cc_g" = "yes"], [
      CFLAGS="-g -W -Wall -Wextra -Wunused -Wshadow -Wpointer-arith -Wmissing-prototypes -Winline -Wcast-align -Wmissing-declarations -Wredundant-decls -Wno-long-long"
    ], [
      CFLAGS=
    ])
  ])
  case "$host" in
    *-linux-gnu)
      AC_CHECK_LIB([bfd], [bfd_init], [
      AC_CHECK_HEADERS([bfd.h], [
      have_bfd=yes
      ], [
      AC_WARN([debug mode is enabled but bfd.h is missing; disabling])
      have_bfd=no
      ])
      ], [
      AC_WARN([debug mode is enabled but libbfd is missing; disabling])
      have_bfd=no
      ])
      ;;
  esac
])

# --with-openal
AC_MSG_CHECKING([whether to use OpenAL])
AC_ARG_WITH([openal],
  AC_HELP_STRING([--with-openal],
     [use the OpenAL sound backend (default: yes)]), [
    AS_IF([test "$withval" = "yes"], [
      with_openal=yes
    ], [
      with_openal=no
    ])
  ], [
    with_openal=yes
  ])
AC_MSG_RESULT([$with_openal])

# --with-sdlmixer
AC_MSG_CHECKING([whether to use SDL_mixer])
AC_ARG_WITH([sdlmixer],
  AC_HELP_STRING([--with-sdlmixer],
     [use the SDL_mixer sound backend (default: yes)]), [
    AS_IF([test "$withval" = "yes"], [
      with_sdlmixer=yes
    ], [
      with_sdlmixer=no
    ])
  ], [
    with_sdlmixer=yes
  ])
AC_MSG_RESULT([$with_sdlmixer])

# --with-ndata
ndata_path="$datadir/$PACKAGE_NAME/ndata"
expand_variable ndata_path

AC_MSG_CHECKING([for path to ndata])
AC_ARG_WITH([ndata-path],
  AC_HELP_STRING([--with-ndata-path=PATH], [path to the ndata file]),
  [ndata_path="$withval"], [])
AC_MSG_RESULT([$ndata_path])
AS_IF([test -z "$ndata_path" || test "$ndata_path" = "yes"], [
  AC_ERROR([The data path must be explicitly specified])
])
AC_SUBST([ndata_path])

# --with-preferences-dir
prefsdir=".$PACKAGE_NAME"
expand_variable prefsdir

AC_MSG_CHECKING([for preferences directory])
AC_ARG_WITH([preferences-dir],
  AC_HELP_STRING([--with-preferences-dir=DIR], [preferences directory]),
  [prefsdir="$withval"], [])
AC_MSG_RESULT([$prefsdir])
AS_IF([test -z "$prefsdir" || test "$prefsdir" = "yes"], [
  AC_ERROR([The preferences directory must be explicitly specified])
])
AC_SUBST([prefsdir])

#
# Checks for libraries.
#

# SDL
AM_PATH_SDL
AS_IF([test -z "$SDL_CFLAGS" && test -z "$SDL_LIBS"], [
  AC_ERROR([SDL not found])
])

# libxml2
PKG_CHECK_MODULES([XML], [libxml-2.0], [], [
  AC_ERROR([libxml-2.0 not found])
])

# FreeType
PKG_CHECK_MODULES([FREETYPE], [freetype2], [], [
  AC_ERROR([freetype2 not found])
])

# vorbisfile
PKG_CHECK_MODULES([VORBISFILE], [vorbisfile], [], [
  AC_ERROR([vorbisfile not found])
])

# libpng
PKG_CHECK_MODULES([PNG], [libpng], [], [
  AC_ERROR([libpng not found])
])

# OpenAL
AS_IF([test "$with_openal" = "yes"], [
  AM_PATH_OPENAL([have_openal=yes], [have_openal=no])
])

# SDL_mixer
AS_IF([test "$with_sdlmixer" = "yes"], [
  VAR_SAVE([LIBS])
  AC_SEARCH_LIBS([Mix_OpenAudio], [SDL_mixer],
     [have_sdlmixer=yes], [have_sdlmixer=no])
  AS_IF([test "$have_sdlmixer" = "yes"], [
    SDLMIXER_CFLAGS=
    SDLMIXER_LIBS="$LIBS"
  ], [
    SDLMIXER_CFLAGS=
    SDLMIXER_LIBS=
  ])
  VAR_RESTORE([LIBS])
], [
  have_sdlmixer=no
])

# SDL_image
#VAR_SAVE([LIBS])
#AC_SEARCH_LIBS([IMG_Load], [SDL_image], [], [
#  AC_ERROR([SDL_image not found])
#])
#SDLIMAGE_CFLAGS=
#SDLIMAGE_LIBS="$LIBS"
#VAR_RESTORE([LIBS])
#AC_SUBST([SDLIMAGE_CFLAGS])
#AC_SUBST([SDLIMAGE_LIBS])

# OpenGL
AM_PATH_OPENGL([], [
  AC_MSG_ERROR([OpenGL not found])
])

#
# Compiler flags
#

# global
GLOBAL_CFLAGS="-include \"\$(top_builddir)/src/config.h\""
GLOBAL_CFLAGS="$GLOBAL_CFLAGS -I\"\$(top_srcdir)/src\""
GLOBAL_LIBS=

AS_IF([test "$enable_debug" != "no"], [
  GLOBAL_CFLAGS="$GLOBAL_CFLAGS -DLUA_USE_APICHECK"
])

case "$host" in
  *-mingw*)
    GLOBAL_LIBS="$NAEV_LIBS -lwsock32"
    ;;
esac

AC_SUBST([GLOBAL_CFLAGS])
AC_SUBST([GLOBAL_LIBS])

# naev
WIDGET_LIBS="\$(top_builddir)/src/tk/widget/libwidget.a"

CSPARSE_CFLAGS="-I\"\$(top_srcdir)/lib/csparse\""
CSPARSE_LIBS="\$(top_builddir)/lib/csparse/libcsparse.a"

LUA_CFLAGS="-I\"\$(top_srcdir)/lib/lua\""
LUA_LIBS="\$(top_builddir)/lib/lua/liblua.a"

NAEV_CFLAGS="$GLOBAL_CFLAGS"
NAEV_LIBS="$GLOBAL_LIBS $WIDGET_LIBS"

AS_IF([test "$enable_debug" != "no"], [
  AC_DEFINE([DEBUGGING], 1, [Define to 1 to enable debug code])
  AC_DEFINE([DEBUG], 1, [Define to 1 to enable debug code])
  AS_IF([test "$have_bfd" != "no"], [
     case "$host" in
       *-linux-gnu)
         AC_DEFINE([HAS_BFD], 1, [Define to 1 to enable bfd backtrace code])
         NAEV_LIBS="$NAEV_LIBS -lbfd -rdynamic"
         ;;
     esac
  ])
])
AS_IF([test "$enable_debug" = "paranoid"], [
  AC_DEFINE([DEBUG_PARANOID], 1, [Define to 1 to enable paranoid debug code])
])

NAEV_CFLAGS="$NAEV_CFLAGS $CSPARSE_CFLAGS $SDL_CFLAGS"
NAEV_CFLAGS="$NAEV_CFLAGS $XML_CFLAGS $FREETYPE_CFLAGS $LUA_CFLAGS"
NAEV_CFLAGS="$NAEV_CFLAGS $VORBISFILE_CFLAGS $OPENGL_CFLAGS $PNG_CFLAGS"
NAEV_CFLAGS="$NAEV_CFLAGS $SDLIMAGE_CFLAGS"

NAEV_LIBS="$NAEV_LIBS $CSPARSE_LIBS $SDL_LIBS $XML_LIBS"
NAEV_LIBS="$NAEV_LIBS $FREETYPE_LIBS $LUA_LIBS"
NAEV_LIBS="$NAEV_LIBS $VORBISFILE_LIBS $OPENGL_LIBS $PNG_LIBS"
NAEV_LIBS="$NAEV_LIBS $SDLIMAGE_LIBS"

AS_IF([test "$have_openal" = "yes"], [
  NAEV_CFLAGS="$NAEV_CFLAGS $OPENAL_CFLAGS"
  NAEV_LIBS="$NAEV_LIBS $OPENAL_LIBS"
])

AS_IF([test "$have_sdlmixer" = "yes"], [
  NAEV_CFLAGS="$NAEV_CFLAGS $SDLMIXER_CFLAGS"
  NAEV_LIBS="$NAEV_LIBS $SDLMIXER_LIBS"
])

case "$host" in
  *-mingw*)
    NAEV_LIBS="$NAEV_LIBS -mwindows"
    ;;
esac

# This is a list of in-tree static libs that naev depends on.  It is used by
# automake to detect when the binary needs recompiling after a library has been
# changed.
NAEV_DEPENDENCIES="$WIDGET_LIBS $LUA_LIBS $CSPARSE_LIBS"

AC_SUBST([NAEV_CFLAGS])
AC_SUBST([NAEV_LIBS])
AC_SUBST([NAEV_DEPENDENCIES])

# liblua
LIBLUA_CFLAGS="$GLOBAL_CFLAGS"

case "$host" in
  *-mingw*)
    LIBLUA_CFLAGS="$LIBLUA_CFLAGS -DLUA_ANSI"
    ;;
  *)
    LIBLUA_CFLAGS="$LIBLUA_CFLAGS -DLUA_USE_POSIX"
    ;;
esac

AC_SUBST([LIBLUA_CFLAGS])


PACK_CFLAGS="$GLOBAL_CFLAGS $SDL_CFLAGS $SDLIMAGE_CFLAGS -DNOLOGPRINTFCONSOLE"
PACK_LIBS="$GLOBAL_LIBS $SDL_LIBS $SDLIMAGE_LIBS"

AC_SUBST([PACK_CFLAGS])
AC_SUBST([PACK_LIBS])

#
# Checks for headers
#
AC_CHECK_HEADERS([fenv.h])

#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

#
# Checks for functions
#
VAR_SAVE([LIBS], ["-lm"])
AC_CHECK_FUNCS([feenableexcept])
VAR_RESTORE([LIBS])

AC_CHECK_FUNCS([clock_gettime])
# if it is not available directly, check it in librt (glibc does this).
if test $ac_cv_func_clock_gettime = no; then
AC_CHECK_LIB(rt, clock_gettime,
      [LIBS="-lrt $LIBS"])
fi

VAR_SAVE([CFLAGS], ["$VORBISFILE_CFLAGS"])
VAR_SAVE([LIBS],   ["$VORBISFILE_LIBS"])
AC_CHECK_FUNCS([ov_read_filter])
VAR_RESTORE([CFLAGS])
VAR_RESTORE([LIBS])

#
# Misc checks
#
AC_MSG_CHECKING([for the current Git commit identifier])
AS_IF([test -n "$GIT" && test -d "$srcdir/.git"], [
  GIT_COMMIT=$(cd "$srcdir" && "$GIT" describe --always --tags)
  AC_MSG_RESULT([$GIT_COMMIT])
], [
  GIT_COMMIT=
  AC_MSG_RESULT([unknown])
])

#
# Defines in config.h
#

# Version variables
VMAJOR=$(echo "$VERSION" | cut -d. -f1)
VMINOR=$(echo "$VERSION" | cut -d. -f2)
VREV=$(echo "$VERSION" | cut -d. -f3)

AC_DEFINE_UNQUOTED([VMAJOR], [$VMAJOR], [Define to the major version component])
AC_DEFINE_UNQUOTED([VMINOR], [$VMINOR], [Define to the minor version component])
AC_DEFINE_UNQUOTED([VREV], [$VREV], [Define to the revision version component])

# USE_OPENAL
AS_IF([test "$have_openal" = "yes"], [
  AC_DEFINE([USE_OPENAL], 1, [Define to 1 if using OpenAL])
])

# USE_SDLMIX
AS_IF([test "$have_sdlmixer" = "yes"], [
  AC_DEFINE([USE_SDLMIX], 1, [Define to 1 if using SDL_mixer])
])

# Operating system
case "$host" in
  *-linux*)
    AC_DEFINE([LINUX], 1, [Define to 1 if running on Linux])
    ;;
  *-freebsd*)
    AC_DEFINE([FREEBSD], 1, [Define to 1 if running on FreeBSD])
    ;;
  *-mingw*)
    AC_CHECK_TOOL(WINDRES, windres) 
    AC_DEFINE([WIN32], 1, [Define to 1 if running on Windows])
    ;;
  *-darwin*)
    AC_DEFINE([MACOSX], 1, [Define to 1 if running on Mac OS X])
    ;;
esac

# NDATA_DEF
AC_DEFINE_UNQUOTED([NDATA_DEF], ["$ndata_path"],
      [Define to the path to the data file])

# PREFSDIR_DEF
AC_DEFINE_UNQUOTED([PREFSDIR_DEF], ["$prefsdir"],
      [Define to preferences directory])

# GIT_COMMIT
AS_IF([test -n "$GIT_COMMIT"], [
  AC_DEFINE_UNQUOTED([GIT_COMMIT], ["$GIT_COMMIT"],
     [Define to the current Git commit identifier])
])

#
# Automake conditionals
#

AM_CONDITIONAL([HAVE_WINDRES], [test -n "$WINDRES"])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_CONDITIONAL([HAVE_LUADOC], [test -n "$LUADOC"])
AM_CONDITIONAL([HAVE_UTILS], [test "$have_utils" = "yes"])
AM_CONDITIONAL([HAVE_DOCS], [test "$have_docs" = "yes"])

#
# Output
#
SHAVE_INIT([build], [enable])
AC_CONFIG_FILES([Makefile
      lib/Makefile
      lib/csparse/Makefile
      lib/lua/Makefile
      src/Makefile
      src/tk/Makefile
      src/tk/widget/Makefile
      build/shave])
AS_IF([test "$have_utils" = "yes"], [
  AC_CONFIG_FILES([utils/Makefile
     utils/pack/Makefile])
])
AS_IF([test "$have_docs" = "yes"], [
  AC_CONFIG_FILES([docs/Makefile])
])

AC_OUTPUT

echo
AC_MSG_NOTICE([prefix:      $prefix])
AC_MSG_NOTICE([ndata:       $ndata_path])
AC_MSG_NOTICE([prefsdir:    $prefsdir])
echo
AC_MSG_NOTICE([sound backends:])
AC_MSG_NOTICE([  OpenAL:    $have_openal])
AC_MSG_NOTICE([  SDL_mixer: $have_sdlmixer])
echo
AC_MSG_NOTICE([debug mode:  $enable_debug])
AS_IF([test "$enable_debug" != "no"], [
   case "$host" in
     *-linux*)
       AC_MSG_NOTICE([  bfd:       $have_bfd])
       ;;
   esac
])

