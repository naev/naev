/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stdint.h>
/** @endcond */

#include "attributes.h"

SENTINEL( 0 )
int _nfile_concatPaths( char buf[static 1], int maxLength,
                        const char path[static 1], ... );
/**
 * @brief Concatenates paths. The result is always NULL terminated.
 *
 *    @param buf Location paths will be copied to.
 *    @param maxLength Length of the allocated buffer. No more than this many
 * characters will be copied.
 *    @param path First component of the path.
 *    @param ... Rest of the path components to be contacenated.
 *    @return The length of the concatenated path on success. -1 on error.
 */
#define nfile_concatPaths( buf, maxLength, path, ... )                         \
   _nfile_concatPaths( buf, maxLength, path, ##__VA_ARGS__, NULL )

const char *nfile_configPath( void );
const char *nfile_cachePath( void );

int   nfile_dirMakeExist( const char *path );
int   nfile_dirExists( const char *path );
int   nfile_fileExists( const char *path ); /* Returns 1 on exists */
int   nfile_backupIfExists( const char *path );
int   nfile_copyIfExists( const char *path1, const char *path2 );
char *nfile_readFile( size_t *filesize, const char *path );
int   nfile_touch( const char *path );
int   nfile_writeFile( const char *data, size_t len, const char *path );
int   nfile_isSeparator( uint32_t c );
