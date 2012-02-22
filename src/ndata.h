/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NDATA_H
#  define NDATA_H


#include <stdint.h>

#include "SDL.h"


/*
 * ndata open/close
 */
int ndata_open (void);
void ndata_close (void);

/*
 * General.
 */
int ndata_check( const char* path );
int ndata_setPath( const char* path );
const char* ndata_getDirname(void);
const char* ndata_getPath (void);
const char* ndata_name (void);

/*
 * Individual file functions.
 */
void* ndata_read( const char* filename, uint32_t *filesize );
char** ndata_list( const char *path, uint32_t* nfiles );
void ndata_sortName( char **files, uint32_t nfiles );


/*
 * RWops.
 */
SDL_RWops *ndata_rwops( const char* filename );


#endif /* NDATA_H */

