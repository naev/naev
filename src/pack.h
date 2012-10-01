/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef PACK_H
#  define PACK_H


#include "ncompat.h"

#include "SDL.h"


#include <fcntl.h> /* creat() and friends */
#include <stdint.h> /* uint32_t */
#if HAS_POSIX
#include <sys/types.h> /* ssize_t */
#endif /* HAS_POSIX */


struct Packfile_s;
typedef struct Packfile_s Packfile_t;

struct Packcache_s;
typedef struct Packcache_s Packcache_t;

/*
 * packcache manipulation.
 */
Packcache_t* pack_openCache( const char* packfile );
void pack_closeCache( Packcache_t* cache );
int pack_checkCache( const Packcache_t* cache, const char* filename );
Packfile_t* pack_openFromCache( Packcache_t* cache, const char* filename );

/*
 * packfile manipulation, automatically alloced and freed (with open and close)
 */
/* basic */
int pack_check( const char* filename );
int pack_files( const char* outfile, const char** infiles, const uint32_t nfiles );
Packfile_t* pack_open( const char* packfile, const char* filename );
ssize_t pack_read( Packfile_t* file, void* buf, const size_t count );
off_t pack_seek( Packfile_t* file, off_t offset, int whence);
long pack_tell( Packfile_t* file );
int pack_close( Packfile_t* file );
/* fancy */
void* pack_readfile( const char* packfile, const char* filename, uint32_t *filesize );
char** pack_listfiles( const char* packfile, uint32_t* nfiles );
void* pack_readfileCached( Packcache_t* cache, const char* filename, uint32_t *filesize );
const char** pack_listfilesCached( Packcache_t* cache, uint32_t* nfiles );

/*
 * for rwops.
 */
SDL_RWops *pack_rwops( const char* packfile, const char* filename );
SDL_RWops *pack_rwopsCached( Packcache_t* cache, const char* filename );


#endif /* PACK_H */

