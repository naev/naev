/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NFILE_H
#  define NFILE_H


char* nfile_basePath (void);
int nfile_dirMakeExist( char* path );
char** nfile_readDir( int* nfiles, char* path );


#endif /* NFILE_H */

