

#ifndef PACK_H
#  define PACK_H


#include <fcntl.h> /* creat() and friends */
#ifndef _POSIX_SOURCE /* not POSIX */
#include <stdio.h>
#endif /* not _POSIX_SOURCE */
#include <stdint.h> /* uint32_t */


typedef struct {
#ifdef _POSIX_SOURCE
	int fd; /* file descriptor */
#else /* not _POSIX_SOURCE */
	FILE* fp;
#endif /* _POSIX_SOURCE */
	uint32_t pos; /* position */
	uint32_t start, end; /* file limits */
} Packfile;


/*
 * packfile manipulation, automatically alloced and freed (with open and close)
 */
/* basic */
int pack_check( const char* filename );
int pack_files( const char* outfile, const char** infiles, const uint32_t nfiles );
int pack_open( Packfile* file, const char* packfile, const char* filename );
ssize_t pack_read( Packfile* file, void* buf, const size_t count );
int pack_close( Packfile* file );
/* fancy */
void* pack_readfile( const char* packfile, const char* filename, uint32_t *filesize );
char** pack_listfiles( const char* packfile, uint32_t* nfiles );


#endif /* PACK_H */

