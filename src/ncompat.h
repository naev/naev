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
#if (defined(linux) || defined(__linux) || defined(__linux__))
#define HAS_LINUX 1
#else
#define HAS_LINUX 0
#endif
/**
 * @brief System is FreeBSD.
 */
#if (defined(__FreeBSD__))
#define HAS_FREEBSD 1
#else
#define HAS_FREEBSD 0
#endif
/**
 * @brief System is Windows-class.
 */
#if (defined(_WIN32))
#define HAS_WIN32 1
#else
#define HAS_WIN32 0

#endif
/**
 * @brief System is Mac OS X.
 */
#if (defined(__APPLE__) && defined(__MACH__))
#define HAS_MACOSX 1
#else
#define HAS_MACOSX 0
#endif


/* Standard specific. */
/**
 * @brief Whether or not the system follows unix standards like $HOME.
 *
 * @note Mac OS X does not define these macros, but does follow unix somewhat.
 */
#if (defined(__unix__) || defined(__unix) || HAS_MACOSX)
#define HAS_UNIX 1
#else
#define HAS_UNIX 0
#endif
/**
 * @brief Whether or not the system is compliant to POSIX.1.
 *
 * @note Most systems don't actually follow it fully so they don't declare that
 *       they support it.  We consider unix good enough.
 */
#if HAS_UNIX
#define HAS_POSIX 1
#else
#define HAS_POSIX 0
#endif


/*
 * Endianness.
 */
/**
 * @brief Whether or not the system is big endian.
 */
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
#define HAS_BIGENDIAN 1
#else
#define HAS_BIGENDIAN 0
#endif
/**
 * @brief Whether or not the system is little endian.
 */
#if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
#define HAS_LILENDIAN 1
#else
#define HAS_LILENDIAN 0
#endif


/* Misc stuff - mainly for debugging. */
/**
 * @brief Whether or not to use filedescriptors.
 */
#if HAS_POSIX
#define HAS_FD 1
#else
#define HAS_FD 0
#endif


#endif /* NCOMPAT_H */

