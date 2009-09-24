#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([naev], [0.4.0], [bobbens@gmail.com])
AC_CONFIG_SRCDIR([src/naev.c])
AC_CONFIG_HEADERS([src/config.h])
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

#
# Checks for programs.
#
AC_PROG_CC
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

AC_CHECK_PROG([CUT], [cut], [cut])
AS_IF([test -z "$CUT"],[
  AC_ERROR([cut not found])
])

AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen])
AC_CHECK_PROG([LUADOC], [luadoc], [luadoc])

#
# Checks for build environment
#
AC_MSG_CHECKING([whether we are building utils])
AS_IF([test -d "utils"], [
  have_utils="yes"
], [
  have_utils="no"
])
AC_MSG_RESULT([$have_utils])

AC_MSG_CHECKING([whether we are have docs])
AS_IF([test -d "docs"], [
  have_docs="yes"
], [
  have_docs="no"
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

have_posix=yes
have_windows=no
have_darwin=no

case "$host" in
  *-mingw*)
    have_windows=yes
    have_posix=no
    ;;
  *-darwin*)
    have_darwin=yes
    ;;
esac

# Fix missing prefix
AS_IF([test "$prefix" = "NONE"], [
  prefix="$ac_default_prefix"
])
AS_IF([test "$exec_prefix" = "NONE"], [
  exec_prefix="$prefix"
])

#
# Checks for command-line arguments
#

# --enable-debug
AC_MSG_CHECKING([whether to enable additional debugging options])
AC_ARG_ENABLE([debug],
  AC_HELP_STRING([--enable-debug=@<:@no|yes|paranoid@:>@],
		 [enable additional debugging options (default: yes)]), [
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

#
# Checks for libraries.
#

# SDL
AM_PATH_SDL

# libxml2
PKG_CHECK_MODULES([XML], [libxml-2.0], [], [
  AC_ERROR([libxml-2.0 not found])
])

# FreeType
PKG_CHECK_MODULES([FREETYPE], [freetype2], [], [
  AC_ERROR([freetype2 not found])
])

# vorbisfile
PKG_CHECK_MODULES([VORBISFILE], [vorbisfile >= 1.2.1], [], [
  AC_ERROR([vorbisfile >= 1.2.1 not found])
])

# libpng
PKG_CHECK_MODULES([PNG], [libpng], [], [
  AC_ERROR([libpng not found])
])

# OpenAL
AS_IF([test "$with_openal" = "yes"], [
  AS_IF([test "$have_darwin" = "yes"], [
    have_openal="yes"
    OPENAL_AL_H="<OpenAL/al.h>"
    OPENAL_ALC_H="<OpenAL/alc.h>"
    OPENAL_CFLAGS=
    OPENAL_LIBS="-framework OpenAL"
  ], [
    AS_IF([test "$have_windows" = "yes"], [
      have_openal="yes"
      OPENAL_AL_H="<OpenAL/al.h>"
      OPENAL_ALC_H="<OpenAL/alc.h>"
      OPENAL_CFLAGS=
      OPENAL_LIBS="-lOpenAL32"
    ], [
      PKG_CHECK_MODULES([OPENAL], [openal], [have_openal=yes], [have_openal=no])

      AS_IF([test "$have_openal" = "no"], [
	AC_ERROR([OpenAL not found])
      ])

      OPENAL_AL_H="<AL/al.h>"
      OPENAL_ALC_H="<AL/alc.h>"
    ])
  ])
  AC_DEFINE_UNQUOTED([OPENAL_AL_H], [$OPENAL_AL_H],
		     [Define to the equivalent of <AL/al.h> on your system])
  AC_DEFINE_UNQUOTED([OPENAL_ALC_H], [$OPENAL_ALC_H],
		     [Define to the equivalent of <AL/alc.h> on your system])
], [
  have_openal=no
])

# SDL_mixer
AS_IF([test "$with_sdlmixer" = "yes"], [
  OLD_LIBS="$LIBS"
  LIBS=""
  AC_SEARCH_LIBS([Mix_OpenAudio], [SDL_mixer],
		 [have_sdlmixer=yes], [have_sdlmixer=no])
  AS_IF([test "$have_sdlmixer" = "yes"], [
    SDLMIXER_CFLAGS=
    SDLMIXER_LIBS="$LIBS"
  ], [
    SDLMIXER_CFLAGS=
    SDLMIXER_LIBS=
  ])
  LIBS="$OLD_LIBS"
], [
  have_sdlmixer=no
])

# SDL_image
OLD_LIBS="$LIBS"
LIBS=""
AC_SEARCH_LIBS([IMG_Load], [SDL_image], [], [
  AC_ERROR([SDL_image not found])
])
SDLIMAGE_CFLAGS=
SDLIMAGE_LIBS="$LIBS"
LIBS="$OLD_LIBS"
AC_SUBST([SDLIMAGE_CFLAGS])
AC_SUBST([SDLIMAGE_LIBS])

# OpenGL
AS_IF([test "$have_darwin" = "yes"], [
  OPENGL_GL_H="<OpenGL/al.h>"
  OPENGL_GLU_H="<OpenGL/alc.h>"
  OPENGL_CFLAGS=
  OPENGL_LIBS="-framework OpenGL"
], [
  OPENGL_CFLAGS=

  OLD_LIBS="$LIBS"
  LIBS=""
  AC_SEARCH_LIBS([glGenTextures], [OpenGL32 GL], [], [
    AC_ERROR([OpenGL not found])
  ])
  AC_SEARCH_LIBS([gluOrtho2D], [GLU32 GLU], [], [
    AC_ERROR([GLU not found])
  ])
  OPENGL_LIBS="$LIBS"
  LIBS="$OLD_LIBS"

  OPENGL_GL_H="<GL/al.h>"
  OPENGL_GLU_H="<GL/glu.h>"
])

AC_DEFINE_UNQUOTED([OPENGL_GL_H], [$OPENGL_GL_H],
                   [Define to the equivalent of <GL/gl.h> on your system])
AC_DEFINE_UNQUOTED([OPENGL_GLU_H], [$OPENGL_GLU_H],
                   [Define to the equivalent of <GL/glu.h> on your system])

#
# Compiler flags
#

# global
GLOBAL_CFLAGS="-include \"\$(top_builddir)/src/config.h\""
GLOBAL_CFLAGS="$GLOBAL_CFLAGS -I\"\$(top_srcdir)/src\""

AS_IF([test "$enable_debug" = "yes"], [
  GLOBAL_CFLAGS="$GLOBAL_CFLAGS -DLUA_USE_APICHECK"
])

# naev
NAEV_CFLAGS=""
NAEV_LIBS="\$(top_builddir)/src/tk/widget/libwidget.a"

AS_IF([test "$enable_debug" != "no"], [
  AC_DEFINE([DEBUGGING], 1, [Define to 1 to enable debug code])
  AC_DEFINE([DEBUG], 1, [Define to 1 to enable debug code])
  case "$host" in
    *-linux-gnu)
      NAEV_LIBS="$NAEV_LIBS -lbfd"
      ;;
  esac
])
AS_IF([test "$enable_debug" = "paranoid"], [
  AC_DEFINE([DEBUG_PARANOID], 1, [Define to 1 to enable paranoid debug code])
])

CSPARSE_CFLAGS="-I\"\$(top_srcdir)/lib/csparse\""
CSPARSE_LIBS="\$(top_builddir)/lib/csparse/libcsparse.a"

LUA_CFLAGS="-I\"\$(top_srcdir)/lib/lua\""
LUA_LIBS="\$(top_builddir)/lib/lua/liblua.a"

NAEV_CFLAGS="$NAEV_CFLAGS $GLOBAL_CFLAGS $CSPARSE_CFLAGS $SDL_CFLAGS"
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

AS_IF([test "$have_windows" = "yes"], [
  NAEV_CFLAGS="$NAEV_CFLAGS -mwindows"
])

AC_SUBST([NAEV_CFLAGS])
AC_SUBST([NAEV_LIBS])

# liblua
LIBLUA_CFLAGS="$GLOBAL_CFLAGS"

AS_IF([test "$have_posix" = "yes"], [
  LIBLUA_CFLAGS="$LIBLUA_CFLAGS -DLUA_USE_POSIX"
], [
  LIBLUA_CFLAGS="$LIBLUA_CFLAGS -DLUA_ANSI"
])

AC_SUBST([LIBLUA_CFLAGS])

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
OLD_LIBS="$LIBS"
LIBS="-lm"
AC_CHECK_FUNCS([feenableexcept])
LIBS="$OLD_LIBS"

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
    AC_DEFINE([WIN32], 1, [Define to 1 if running on Windows])
    ;;
  *-darwin*)
    AC_DEFINE([MACOSX], 1, [Define to 1 if running on Mac OS X])
    ;;
esac

# NDATA_DEF
AC_DEFINE_UNQUOTED([NDATA_DEF], ["$ndata_path"],
		   [Define to the path to the data file])

#
# Automake conditionals
#

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
echo
AC_MSG_NOTICE([sound backends:])
AC_MSG_NOTICE([  OpenAL:    $have_openal])
AC_MSG_NOTICE([  SDL_mixer: $have_sdlmixer])
echo
AC_MSG_NOTICE([debug mode:  $enable_debug])
