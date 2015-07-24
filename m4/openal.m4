# Configure headers/flags for OpenAL
# Unavowed <unavowed@vexillium.org>

dnl AM_PATH_OPENAL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl AC_DEFINE OPENAL_AL_H/OPENAL_ALC_H to the equivalent of
dnl <AL/al.h>/<AL/alc.h> so you can do #include OPENAL_AL_H.  Define
dnl OPENAL_CFLAGS and OPENAL_LIBS.
AC_DEFUN([AM_PATH_OPENAL], [
  OPENAL_CFLAGS=
  OPENAL_LIBS=

  # First check for headers
  for header in "AL/al.h" "OpenAL/al.h"; do
    AC_CHECK_HEADER([$header], [
      ac_cv_openal_al_h="$header"
      break
    ])
  done
  AS_IF([test -n "$ac_cv_openal_al_h"], [
    ac_cv_openal_alc_h=$(echo "$ac_cv_openal_al_h" | sed 's/al\.h$/alc.h/')
    AC_DEFINE_UNQUOTED([OPENAL_AL_H], [<$ac_cv_openal_al_h>],
		       [Define to the equivalent of <AL/al.h> on your system])
    AC_DEFINE_UNQUOTED([OPENAL_ALC_H], [<$ac_cv_openal_alc_h>],
		       [Define to the equivalent of <AL/alc.h> on your system])
  ])

  # Then check for libs
  ac_cv_openal_al_libs=
  AS_IF([test -n "$ac_cv_openal_al_h" && test -n "$ac_cv_openal_alc_h"], [
    OLD_LIBS="$LIBS"
    for lib in "-framework OpenAL" "-lopenal" "-lopenal32"; do
      LIBS="$OLD_LIBS $lib"
      AC_MSG_CHECKING([for alGenSources in $lib])
      AC_TRY_LINK([#include OPENAL_AL_H], [alGenSources (1, 0);], [
	ac_cv_openal_al_libs="$lib"
	AC_MSG_RESULT([yes])
	break
      ], [
	AC_MSG_RESULT([no])
      ])
    done

    LIBS="$OLD_LIBS"
    OPENAL_LIBS="$ac_cv_openal_al_libs"
  ])

  AC_SUBST([OPENAL_CFLAGS])
  AC_SUBST([OPENAL_LIBS])

  AS_IF([test -z "$OPENAL_CFLAGS" && test -z "$OPENAL_LIBS"], [$2], [$1])
])
