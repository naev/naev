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
      ac_cv_opengl_gl_h="$header"
      break
    ])
  done
  for header in "GL/glu.h" "OpenGL/glu.h"; do
    AC_CHECK_HEADER([$header], [
      ac_cv_opengl_glu_h="$header"
      break
    ])
  done
  AS_IF([test -n "$ac_cv_opengl_gl_h"], [
    AC_DEFINE_UNQUOTED([OPENGL_GL_H], [<$ac_cv_opengl_gl_h>],
		       [Define to the equivalent of <GL/gl.h> on your system])
  ])
  AS_IF([test -n "$ac_cv_opengl_glu_h"], [
    AC_DEFINE_UNQUOTED([OPENGL_GLU_H], [<$ac_cv_opengl_glu_h>],
		       [Define to the equivalent of <GL/glu.h> on your system])
  ])

  # Then check for libs
  ac_cv_opengl_gl_libs=
  ac_cv_opengl_glu_libs=
  AS_IF([test -n "$ac_cv_opengl_gl_h" && test -n "$ac_cv_opengl_glu_h"], [
    OLD_LIBS="$LIBS"
    for lib in "-framework OpenGL" "-lGL" "-lopengl32"; do
      LIBS="$OLD_LIBS $lib"
      AC_MSG_CHECKING([for glGenTextures in $lib])
      AC_TRY_LINK([#include OPENGL_GL_H], [glGenTextures (1, 0);], [
	ac_cv_opengl_gl_libs="$lib"
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
	ac_cv_opengl_glu_libs="$lib"
	AC_MSG_RESULT([yes])
	break
      ], [
	AC_MSG_RESULT([no])
      ])
    done

    LIBS="$OLD_LIBS"

    AS_IF([test "$ac_cv_opengl_gl_libs" != "$ac_cv_opengl_glu_libs"], [
      OPENGL_LIBS="$ac_cv_opengl_gl_libs $ac_cv_opengl_glu_libs"
    ], [
      OPENGL_LIBS="$ac_cv_opengl_gl_libs"
    ])
  ])

  AC_SUBST([OPENGL_CFLAGS])
  AC_SUBST([OPENGL_LIBS])

  AS_IF([test -z "$OPENGL_CFLAGS" && test -z "$OPENGL_LIBS"], [$2], [$1])
])
