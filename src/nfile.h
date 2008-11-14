/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NFILE_H
#  define NFILE_H


char* nfile_basePath (void);
int nfile_dirMakeExist( const char* path, ... ); /* Creates if doesn't exist, 0 success */
int nfile_fileExists( const char* path, ... ); /* Returns 1 on exists */
char** nfile_readDir( int* nfiles, const char* path, ... );


#endif /* NFILE_H */

