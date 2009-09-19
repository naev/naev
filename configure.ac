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
# Checks for programs.
#
AC_PROG_CC
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

AC_CHECK_PROG([CUT], [cut], [cut])
AS_IF([test -z "$CUT"],[
  AC_ERROR([cut not found])
])

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
  *-*-mingw*)
    have_windows=yes
    have_posix=no
    ;;
  *-*-darwin*)
    have_darwin=yes
    ;;
esac

#
# Checks for command-line arguments
#

AC_MSG_CHECKING([whether to enable additional debugging options])
AC_ARG_ENABLE([debug],
  AC_HELP_STRING([--enable-debug=@<:@no|yes|paranoid@:>@],
		 [enable additional debugging options]), [
    case "$enableval" in
      no)
	enable_debug=no
	enable_paranoid_debug=no
	;;
      paranoid)
        enable_debug=yes
	enable_paranoid_debug=yes
        ;;
      *)
	enable_debug=yes
	enable_paranoid_debug=no
	;;
    esac
  ], [
    enable_debug=no
  ])
AC_MSG_RESULT([$enable_debug])

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

# SDL_mixer
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

# SDL_image
OLD_LIBS="$LIBS"
LIBS=""
AC_SEARCH_LIBS([IMG_Load], [SDL_image], [], [
  AC_ERROR([SDL_image not found])
])
SDLIMAGE_CFLAGS=
SDLIMAGE_LIBS="$LIBS"
LIBS="$OLD_LIBS"

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

AS_IF([test "$enable_debug" = "yes"], [
  AC_DEFINE([DEBUGGING], 1, [Define to 1 to enable debug code])
  AC_DEFINE([DEBUG], 1, [Define to 1 to enable debug code])
  case "$host" in
    *-linux-gnu)
      NAEV_LIBS="$NAEV_LIBS -lbfd"
      ;;
  esac
])
AS_IF([test "$enable_paranoid_debug" = "yes"], [
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
NAEV_CFLAGS="$NAEV_CFLAGS $SDLIMAGE_LIBS"

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
# Defines in config.h
#

# Version variables
VMAJOR=$(echo "$VERSION" | cut -d. -f1)
VMINOR=$(echo "$VERSION" | cut -d. -f2)
VREV=$(echo "$VERSION" | cut -d. -f3)

AC_DEFINE_UNQUOTED([VMAJOR], [$VMAJOR], [Define to the major version component])
AC_DEFINE_UNQUOTED([VMINOR], [$VMINOR], [Define to the minor version component])
AC_DEFINE_UNQUOTED([VREV], [$VREV], [Define to the revision version component])

AS_IF([test "$have_openal" = "yes"], [
  AC_DEFINE([USE_OPENAL], 1, [Define to 1 if using OpenAL])
])

AS_IF([test "$have_sdlmixer" = "yes"], [
  AC_DEFINE([USE_SDLMIX], 1, [Define to 1 if using SDL_mixer])
])

SHAVE_INIT([.], [enable])
AC_CONFIG_FILES([Makefile
		 lib/Makefile
		 lib/csparse/Makefile
		 lib/lua/Makefile
		 src/Makefile
		 src/tk/Makefile
		 src/tk/widget/Makefile
		 shave])
AC_OUTPUT
