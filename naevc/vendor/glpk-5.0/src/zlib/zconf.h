/* zconf.h (configuration of the zlib compression library) */

/* Modified by Andrew Makhorin <mao@gnu.org>, April 2011 */

/* Copyright (C) 1995-2010 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in
 * zlib.h */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h. */

#ifndef ZCONF_H
#define ZCONF_H

/* (file adler32.c) */
#define adler32               _glp_zlib_adler32
#define adler32_combine       _glp_zlib_adler32_combine
#define adler32_combine64     _glp_zlib_adler32_combine64

/* (file compress.c) */
#define compress2             _glp_zlib_compress2
#define compress              _glp_zlib_compress
#define compressBound         _glp_zlib_compressBound

/* (file crc32.c) */
#define get_crc_table         _glp_zlib_get_crc_table
#define crc32                 _glp_zlib_crc32
#define crc32_combine         _glp_zlib_crc32_combine
#define crc32_combine64       _glp_zlib_crc32_combine64

/* (file deflate.c) */
#define deflateInit_          _glp_zlib_deflateInit_
#define deflateInit2_         _glp_zlib_deflateInit2_
#define deflateSetDictionary  _glp_zlib_deflateSetDictionary
#define deflateReset          _glp_zlib_deflateReset
#define deflateSetHeader      _glp_zlib_deflateSetHeader
#define deflatePrime          _glp_zlib_deflatePrime
#define deflateParams         _glp_zlib_deflateParams
#define deflateTune           _glp_zlib_deflateTune
#define deflateBound          _glp_zlib_deflateBound
#define deflate               _glp_zlib_deflate
#define deflateEnd            _glp_zlib_deflateEnd
#define deflateCopy           _glp_zlib_deflateCopy
#define deflate_copyright     _glp_zlib_deflate_copyright

/* (file gzclose.c) */
#define gzclose               _glp_zlib_gzclose

/* (file gzlib.c) */
#define gzopen                _glp_zlib_gzopen
#define gzopen64              _glp_zlib_gzopen64
#define gzdopen               _glp_zlib_gzdopen
#define gzbuffer              _glp_zlib_gzbuffer
#define gzrewind              _glp_zlib_gzrewind
#define gzseek64              _glp_zlib_gzseek64
#define gzseek                _glp_zlib_gzseek
#define gztell64              _glp_zlib_gztell64
#define gztell                _glp_zlib_gztell
#define gzoffset64            _glp_zlib_gzoffset64
#define gzoffset              _glp_zlib_gzoffset
#define gzeof                 _glp_zlib_gzeof
#define gzerror               _glp_zlib_gzerror
#define gzclearerr            _glp_zlib_gzclearerr
#define gz_error              _glp_zlib_gz_error

/* (file gzread.c) */
#define gzread                _glp_zlib_gzread
#define gzgetc                _glp_zlib_gzgetc
#define gzungetc              _glp_zlib_gzungetc
#define gzgets                _glp_zlib_gzgets
#define gzdirect              _glp_zlib_gzdirect
#define gzclose_r             _glp_zlib_gzclose_r

/* (file gzwrite.c) */
#define gzwrite               _glp_zlib_gzwrite
#define gzputc                _glp_zlib_gzputc
#define gzputs                _glp_zlib_gzputs
#define gzprintf              _glp_zlib_gzprintf
#define gzflush               _glp_zlib_gzflush
#define gzsetparams           _glp_zlib_gzsetparams
#define gzclose_w             _glp_zlib_gzclose_w

/* (file infback.c) */
#define inflateBackInit_      _glp_zlib_inflateBackInit_
#define inflateBack           _glp_zlib_inflateBack
#define inflateBackEnd        _glp_zlib_inflateBackEnd

/* (file inffast.c) */
#define inflate_fast          _glp_zlib_inflate_fast

/* (file inflate.c) */
#define inflateReset          _glp_zlib_inflateReset
#define inflateReset2         _glp_zlib_inflateReset2
#define inflateInit2_         _glp_zlib_inflateInit2_
#define inflateInit_          _glp_zlib_inflateInit_
#define inflatePrime          _glp_zlib_inflatePrime
#define inflate               _glp_zlib_inflate
#define inflateEnd            _glp_zlib_inflateEnd
#define inflateSetDictionary  _glp_zlib_inflateSetDictionary
#define inflateGetHeader      _glp_zlib_inflateGetHeader
#define inflateSync           _glp_zlib_inflateSync
#define inflateSyncPoint      _glp_zlib_inflateSyncPoint
#define inflateCopy           _glp_zlib_inflateCopy
#define inflateUndermine      _glp_zlib_inflateUndermine
#define inflateMark           _glp_zlib_inflateMark

/* (file inftrees.c) */
#define inflate_table         _glp_zlib_inflate_table
#define inflate_copyright     _glp_zlib_inflate_copyright

/* (file trees.c) */
#define _tr_init              _glp_zlib_tr_init
#define _tr_stored_block      _glp_zlib_tr_stored_block
#define _tr_align             _glp_zlib_tr_align
#define _tr_flush_block       _glp_zlib_tr_flush_block
#define _tr_tally             _glp_zlib_tr_tally
#define _dist_code            _glp_zlib_dist_code
#define _length_code          _glp_zlib_length_code

/* (file uncompr.c) */
#define uncompress            _glp_zlib_uncompress

/* (file zutil.c) */
#define zlibVersion           _glp_zlib_zlibVersion
#define zlibCompileFlags      _glp_zlib_zlibCompileFlags
#define zError                _glp_zlib_zError
#define zcalloc               _glp_zlib_zcalloc
#define zcfree                _glp_zlib_zcfree
#define z_errmsg              _glp_zlib_z_errmsg

#define STDC 1

#define MAX_MEM_LEVEL 9

#define MAX_WBITS 15

#define OF(args) args

#define ZEXTERN extern
#define ZEXPORT
#define ZEXPORTVA

#define FAR

typedef unsigned char Byte;
typedef unsigned int uInt;
typedef unsigned long uLong;

typedef Byte Bytef;
typedef char charf;
typedef int intf;
typedef uInt uIntf;
typedef uLong uLongf;

typedef void const *voidpc;
typedef void *voidpf;
typedef void *voidp;

#define z_off_t long

#define z_off64_t z_off_t

#define NO_vsnprintf 1

#endif

/* eof */
