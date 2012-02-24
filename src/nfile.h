/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NFILE_H
#  define NFILE_H


const char* nfile_dataPath (void);
const char* nfile_configPath (void);
const char* nfile_cachePath (void);
char* nfile_dirname( char *path );
int nfile_dirMakeExist( const char* path, ... ); /* Creates if doesn't exist, 0 success */
int nfile_dirExists( const char* path, ... ); /* Returns 1 on exists. */
int nfile_fileExists( const char* path, ... ); /* Returns 1 on exists */
int nfile_backupIfExists( const char* path, ... );
char** nfile_readDir( int* nfiles, const char* path, ... );
char* nfile_readFile( int* filesize, const char* path, ... );
int nfile_touch( const char* path, ... );
int nfile_writeFile( const char* data, int len, const char* path, ... );
int nfile_delete( const char* file );
int nfile_rename( const char* oldname, const char* newname );


#endif /* NFILE_H */

