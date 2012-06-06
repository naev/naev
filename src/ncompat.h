/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NCOMPAT_H
#  define NCOMPAT_H


#include "SDL.h"


/* System specific. */
/**
 * @brief System is Linux-class.
 */
#define HAS_LINUX    (defined(linux) || defined(__linux) || defined(__linux__))
/**
 * @brief System is FreeBSD.
 */
#define HAS_FREEBSD  (defined(__FreeBSD__))
/**
 * @brief System is Windows-class.
 */
#define HAS_WIN32    (defined(_WIN32))
/**
 * @brief System is Mac OS X.
 */
#define HAS_MACOSX   (defined(__APPLE__) && defined(__MACH__))


/* Standard specific. */
/**
 * @brief Whether or not the system is compliant to POSIX.1.
 *
 * @note Most systems don't actually follow it fully so they don't declare that
 *       they support it.  We consider unix good enough.
 */
#define HAS_POSIX    HAS_UNIX /* (defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200112L)) */
/**
 * @brief Whether or not the system follows unix standards like $HOME.
 *
 * @note Mac OS X does not define these macros, but does follow unix somewhat.
 */
#define HAS_UNIX     (defined(__unix__) || defined(__unix) || HAS_MACOSX)


/*
 * Endianness.
 */
/**
 * @brief Whether or not the system is big endian.
 */
#define HAS_BIGENDIAN (SDL_BYTEORDER == SDL_BIG_ENDIAN)
/**
 * @brief Whether or not the system is little endian.
 */
#define HAS_LILENDIAN (SDL_BYTEORDER == SDL_LIL_ENDIAN)


/* Misc stuff - mainly for debugging. */
/**
 * @brief Whether or not to use filedescriptors.
 */
#define HAS_FD     HAS_POSIX


#endif /* NCOMPAT_H */

