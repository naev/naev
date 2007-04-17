

#ifndef PACK_H
#  define PACK_H


#include <fcntl.h> /* creat() and friends */
#include <stdint.h> /* uint32_t */


typedef struct {
	int fd; /* file descriptor */
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


#endif /* PACK_H */
