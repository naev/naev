/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef PACK_H
#  define PACK_H


#include <fcntl.h> /* creat() and friends */
#ifndef _POSIX_SOURCE /* not POSIX */
#include <stdio.h>
#endif /* not _POSIX_SOURCE */
#include <stdint.h> /* uint32_t */
#include <sys/types.h> /* ssize_t */


struct Packfile_s;
typedef struct Packfile_s Packfile_t;


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


#endif /* PACK_H */

