

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
int pack_check( char* filename );
int pack_files( char* outfile, char** infiles, uint32_t nfiles );
int pack_open( Packfile* file, char* packfile, char* filename );
ssize_t pack_read( Packfile* file, void* buf, size_t count );
int pack_close( Packfile* file );
/* fancy */
void* pack_readfile( char* packfile, char* filename, uint32_t *filesize );


#endif /* PACK_H */
