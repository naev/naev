/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NCOMPAT_H
#  define NCOMPAT_H


/* System specific. */
#define HAS_LINUX    (defined(linux) || defined(__linux) || defined(__linux__))
#define HAS_FREEBSD  (defined(__FreeBSD__))
#define HAS_WIN32    (defined(_WIN32))


/* Standard specific. */
#define HAS_POSIX    HAS_UNIX /* (defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200112L)) */
#define HAS_UNIX     (defined(__unix__) || defined(__unix))


#endif /* NCOMPAT_H */

