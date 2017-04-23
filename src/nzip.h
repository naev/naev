/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef NZIP_H
#define NZIP_H

#ifdef USE_LIBZIP

#include <zip.h>
#include <SDL.h>

int nzip_isZip ( const char* filename );
struct zip* nzip_open ( const char* filename );
void nzip_close ( struct zip* arc );

int nzip_hasFile ( struct zip* arc, const char* filename );
void* nzip_readFile ( struct zip* arc, const char* filename, uint32_t* size );
char** nzip_listFiles ( struct zip* arc, uint32_t* nfiles );

SDL_RWops* nzip_rwops ( struct zip* arc, const char* filename );

#else

#define nzip_isZip(a) 0
#define nzip_open(a) NULL
#define nzip_close(a)
#define nzip_hasFile(a, b) 0
#define nzip_readFile(a, b, c) NULL
#define nzip_listFiles(a, b) NULL
#define nzip_rwops(a, b) NULL

#endif /* USE_LIBZIP */

#endif
