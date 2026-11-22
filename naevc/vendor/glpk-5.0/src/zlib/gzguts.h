/* gzguts.h (zlib internal header definitions for gz* operations) */

/* Modified by Andrew Makhorin <mao@gnu.org>, April 2011 */

/* Copyright (C) 2004, 2005, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in
 * zlib.h */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h. */

#ifndef GZGUTS_H
#define GZGUTS_H

#define ZLIB_INTERNAL

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zio.h"
#include "zlib.h"

#define local static

#define zstrerror() strerror(errno)

#define GZBUFSIZE 8192

#define GZ_NONE 0
#define GZ_READ 7247
#define GZ_WRITE 31153
#define GZ_APPEND 1

#define LOOK 0
#define COPY 1
#define GZIP 2

typedef struct
{     int mode;
      int fd;
      char *path;
      z_off64_t pos;
      unsigned size;
      unsigned want;
      unsigned char *in;
      unsigned char *out;
      unsigned char *next;
      unsigned have;
      int eof;
      z_off64_t start;
      z_off64_t raw;
      int how;
      int direct;
      int level;
      int strategy;
      z_off64_t skip;
      int seek;
      int err;
      char *msg;
      z_stream strm;
} gz_state;

typedef gz_state *gz_statep;

void ZLIB_INTERNAL gz_error OF((gz_statep, int, const char *));

#define GT_OFF(x) (sizeof(int) == sizeof(z_off64_t) && (x) > INT_MAX)

#endif

/* eof */
