/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NFILE_H
#define NFILE_H


#include <stddef.h>
#include <stdint.h>

__attribute__( ( sentinel ) ) int _nfile_concatPaths( char buf[static 1], int maxLength, const char path[static 1], ... );
/**
 * @brief Concatenates paths. The result is always NULL terminated.
 *
 *    @param buf Location paths will be copied to.
 *    @param maxLength Length of the allocated buffer. No more than this many characters will be copied.
 *    @param path First component of the path.
 *    @param ... Rest of the path components to be contacenated.
 *    @return The length of the concatenated path on success. -1 on error.
 */
#define nfile_concatPaths( buf, maxLength, path, ... ) _nfile_concatPaths( buf, maxLength, path, ##__VA_ARGS__, NULL )

#define _nfile_unwrap( ... ) __VA_ARGS__

// clang-format off
#define _nfile_path_macro( func, err_val, params, path, ... ) ({                    \
   #__VA_ARGS__[0] == '\0'                  /* If there's no vaargs... */           \
      ? func(_nfile_unwrap params(path))    /* Call the function directly */        \
      : ({                                  /* Otherwise, concat the paths first */ \
         char combined_path[ PATH_MAX ];                                            \
         nfile_concatPaths(combined_path, PATH_MAX, (path), ##__VA_ARGS__) < 0      \
            ? (err_val)                                                             \
            : func(_nfile_unwrap params combined_path);                             \
      });                                                                           \
   })
// clang-format on


const char *nfile_dataPath( void );
const char *nfile_configPath( void );
const char *nfile_cachePath( void );

const char *_nfile_dirname( const char *path );
#define nfile_dirname( ... ) _nfile_path_macro( _nfile_dirname, NULL, (), ##__VA_ARGS__ )

int _nfile_dirMakeExist( const char *path );
/**
 * @brief Creates a directory if it doesn't exist.
 *
 *    @param path Path to create directory if it doesn't exist.
 *    @return 0 on success.
 */
#define nfile_dirMakeExist( ... ) _nfile_path_macro( _nfile_dirMakeExist, -1, (), ##__VA_ARGS__ )

int _nfile_dirExists( const char *path );
/**
 * @brief Checks to see if a directory exists.
 *
 * @param path Path to directory
 * @return 1 on exists, 0 otherwise
 */
#define nfile_dirExists( ... ) _nfile_path_macro( _nfile_dirExists, 0, (), ##__VA_ARGS__ )

int _nfile_fileExists( const char *path ); /* Returns 1 on exists */
#define nfile_fileExists( ... ) _nfile_path_macro( _nfile_fileExists, 0, (), ##__VA_ARGS__ )

int _nfile_backupIfExists( const char *path );
#define nfile_backupIfExists( ... ) _nfile_path_macro( _nfile_backupIfExists, 0, (), ##__VA_ARGS__ )

int nfile_copyIfExists( const char *path1, const char *path2 );

char **_nfile_readDir( size_t *nfiles, const char *path );
// clang-format off
#define nfile_readDir( nfiles, ... )                                         \
   ({                                                                        \
      *nfiles = 0;                                                           \
      _nfile_path_macro( _nfile_readDir, NULL, ( nfiles, ), ##__VA_ARGS__ ); \
   })
// clang-format on

char **_nfile_readDirRecursive( char ***files, const char *base_dir, const char *sub_dir );
#define nfile_readDirRecursive( base_dir, sub_dir ) _nfile_readDirRecursive( NULL, base_dir, sub_dir )

char *_nfile_readFile( size_t *filesize, const char *path );
#define nfile_readFile( filesize, ... ) _nfile_path_macro( _nfile_readFile, NULL, ( filesize, ), ##__VA_ARGS__ )

int _nfile_touch( const char *path );
#define nfile_touch( ... ) _nfile_path_macro( _nfile_touch, -1, (), ##__VA_ARGS__ )

int _nfile_writeFile( const char *data, size_t len, const char *path );
#define nfile_writeFile( data, len, ... ) _nfile_path_macro( _nfile_writeFile, -1, ( data, len, ), ##__VA_ARGS__ )

int _nfile_delete( const char *file );
#define nfile_delete( ... ) _nfile_path_macro( _nfile_delete, -1, (), ##__VA_ARGS__ )

int nfile_rename( const char *oldname, const char *newname );

int nfile_isSeparator( uint32_t c );


#endif /* NFILE_H */
