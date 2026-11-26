/* zio.h (simulation of non-standard low-level i/o functions) */

/* Written by Andrew Makhorin <mao@gnu.org>, April 2011
 * For conditions of distribution and use, see copyright notice in
 * zlib.h */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h. */

#ifndef ZIO_H
#define ZIO_H

#define O_RDONLY 0x00
#define O_WRONLY 0x01
#define O_CREAT  0x10
#define O_TRUNC  0x20
#define O_APPEND 0x30

#define open _glp_zlib_open
int open(const char *path, int oflag, ...);

#define read _glp_zlib_read
long read(int fd, void *buf, unsigned long nbyte);

#define write _glp_zlib_write
long write(int fd, const void *buf, unsigned long nbyte);

#define lseek _glp_zlib_lseek
long lseek(int fd, long offset, int whence);

#define close _glp_zlib_close
int close(int fd);

#endif

/* eof */
