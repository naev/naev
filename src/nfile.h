/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NFILE_H
#  define NFILE_H


#include <stddef.h>
#include <stdint.h>


const char* nfile_dataPath (void);
const char* nfile_configPath (void);
const char* nfile_cachePath (void);
char* nfile_dirname( char *path );
int nfile_dirMakeExist( const char* path, ... ); /* Creates if doesn't exist, 0 success */
int nfile_dirExists( const char* path, ... ); /* Returns 1 on exists. */
int nfile_fileExists( const char* path, ... ); /* Returns 1 on exists */
int nfile_backupIfExists( const char* path, ... );
int nfile_copyIfExists( const char* path1, const char* path2 );
char** nfile_readDir( size_t* nfiles, const char* path, ... );
char** nfile_readDirRecursive( size_t* nfiles, const char* path, ... );
char* nfile_readFile( size_t* filesize, const char* path, ... );
int nfile_touch( const char* path, ... );
int nfile_writeFile( const char* data, size_t len, const char* path, ... );
int nfile_delete( const char* file );
int nfile_rename( const char* oldname, const char* newname );
int nfile_isSeparator( uint32_t c );


#endif /* NFILE_H */

