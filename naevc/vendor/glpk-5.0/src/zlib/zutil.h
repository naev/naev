/* zutil.h (internal interface of the zlib compression library) */

/* Modified by Andrew Makhorin <mao@gnu.org>, April 2011 */

/* Copyright (C) 1995-2010 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in
 * zlib.h */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h. */

#ifndef ZUTIL_H
#define ZUTIL_H

#define ZLIB_INTERNAL

#include "zlib.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define local static

typedef unsigned char uch;
typedef uch uchf;
typedef unsigned short ush;
typedef ush ushf;
typedef unsigned long ulg;

extern const char * const z_errmsg[10];

#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]

#define ERR_RETURN(strm, err) \
      return (strm->msg = (char *)ERR_MSG(err), (err))

#define DEF_WBITS MAX_WBITS

#if MAX_MEM_LEVEL >= 8
#define DEF_MEM_LEVEL 8
#else
#define DEF_MEM_LEVEL MAX_MEM_LEVEL
#endif

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2

#define MIN_MATCH 3
#define MAX_MATCH 258

#define PRESET_DICT 0x20

#define OS_CODE 0x03 /* assume Unix */

#define HAVE_MEMCPY 1
#define zmemcpy memcpy
#define zmemzero(dest, len) memset(dest, 0, len)

#ifdef DEBUG
#include <stdio.h>
extern int ZLIB_INTERNAL z_verbose;
extern void ZLIB_INTERNAL z_error OF((char *m));
#define Assert(cond, msg) { if(!(cond)) z_error(msg); }
#define Trace(x) { if (z_verbose >= 0) fprintf x; }
#define Tracev(x) { if (z_verbose > 0) fprintf x; }
#define Tracevv(x) {if (z_verbose > 1) fprintf x; }
#define Tracec(c, x) {if (z_verbose > 0 && (c)) fprintf x; }
#define Tracecv(c, x) {if (z_verbose > 1 && (c)) fprintf x; }
#else
#define Assert(cond, msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c, x)
#define Tracecv(c, x)
#endif

voidpf ZLIB_INTERNAL zcalloc OF((voidpf opaque, unsigned items,
      unsigned size));
void ZLIB_INTERNAL zcfree OF((voidpf opaque, voidpf ptr));

#define ZALLOC(strm, items, size) \
      (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr) \
      (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) { if (p) ZFREE(s, p); }

#endif

/* eof */
