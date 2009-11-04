# Configure headers/flags for OpenGL
# Unavowed <unavowed@vexillium.org>

dnl AM_PATH_OPENGL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl AC_DEFINE OPENGL_GL_H/OPENGL_GLU_H to the equivalent of
dnl <GL/gl.h>/<GL/glu.h> so you can do #include OPENGL_GL_H.  Define
dnl OPENGL_CFLAGS and OPENGL_LIBS.
AC_DEFUN([AM_PATH_OPENGL], [
  OPENGL_CFLAGS=
  OPENGL_LIBS=

  # First check for headers
  for header in "GL/gl.h" "OpenGL/gl.h"; do
    AC_CHECK_HEADER([$header], [
      OPENGL_GL_H="$header"
      break
    ])
  done
  for header in "GL/glu.h" "OpenGL/glu.h"; do
    AC_CHECK_HEADER([$header], [
      OPENGL_GLU_H="$header"
      break
    ])
  done
  AS_IF([test -n "$OPENGL_GL_H"], [
    AC_DEFINE_UNQUOTED([OPENGL_GL_H], [<$OPENGL_GL_H>],
		       [Define to the equivalent of <GL/gl.h> on your system])
  ])
  AS_IF([test -n "$OPENGL_GLU_H"], [
    AC_DEFINE_UNQUOTED([OPENGL_GLU_H], [<$OPENGL_GLU_H>],
		       [Define to the equivalent of <GL/glu.h> on your system])
  ])

  # Then check for libs
  OPENGL_GL_LIBS=
  OPENGL_GLU_LIBS=
  AS_IF([test -n "$OPENGL_GL_H" && test -n "$OPENGL_GLU_H"], [
    OLD_LIBS="$LIBS"
    for lib in "-framework OpenGL" "-lGL" "-lopengl32"; do
      LIBS="$OLD_LIBS $lib"
      AC_MSG_CHECKING([for glGenTextures in $lib])
      AC_TRY_LINK([#include OPENGL_GL_H], [glGenTextures (1, 0);], [
	OPENGL_GL_LIBS="$lib"
	AC_MSG_RESULT([yes])
	break
      ], [
	AC_MSG_RESULT([no])
      ])
    done
    for lib in "-framework OpenGL" "-lGLU" "-lglu32"; do
      LIBS="$OLD_LIBS $lib"
      AC_MSG_CHECKING([for glOrtho2D in $lib])
      AC_TRY_LINK([#include OPENGL_GLU_H], [gluOrtho2D (.0, .0, .0, .0);], [
	OPENGL_GLU_LIBS="$lib"
	AC_MSG_RESULT([yes])
	break
      ], [
	AC_MSG_RESULT([no])
      ])
    done

    LIBS="$OLD_LIBS"

    AS_IF([test "$OPENGL_GL_LIBS" != "$OPENGL_GLU_LIBS"], [
      OPENGL_LIBS="$OPENGL_GL_LIBS $OPENGL_GLU_LIBS"
    ], [
      OPENGL_LIBS="$OPENGL_GL_LIBS"
    ])
  ])

  AC_SUBST([OPENGL_CFLAGS])
  AC_SUBST([OPENGL_LIBS])

  AS_IF([test -z "$OPENGL_CFLAGS" && test -z "$OPENGL_LIBS"], [$2], [$1])
])
