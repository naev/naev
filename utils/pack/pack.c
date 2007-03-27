
#include <stdio.h> /* printf() */
#include <stdlib.h> /* exit() */
#include <stdint.h> /* uint32_t */
#include <fcntl.h> /* creat() and friends */
#include <sys/stat.h> /* S_IRUSR */
#include <unistd.h> /* write() */
#include <errno.h> /* error numbers */
#include <string.h> /* strlen() and frieds */


#define USAGE  "Usage is: %s output input\n",argv[0]
#define PERMS	 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH


const uint64_t magic =  0x25524573; /* sER% */

/*
 * prototypes
 */
off_t getfilesize( const char* filename );
int fileexists( const char* filename );


int main( int argc, char** argv )
{
	switch (argc) {
		case 1:
			printf("Missing output file\n");
		case 2:
			printf("Missing input file/s\n\n");
			goto usage;
			break;
	}

	int i, namesize;
	int outfd, infd;
	uint32_t nfiles = (uint32_t)argc - 2;

	for (namesize=0,i=0; i < nfiles; i++) {/* make sure files exist before writing */
		if (!fileexists(argv[i+2])) {
			printf("File %s does not exist\n", argv[i+2]);
			goto failure;
		}
		namesize += strlen(argv[i+2]);
	}

	/* creates the output file */
	outfd = creat( argv[1], PERMS );
	if (outfd == -1) goto failure;

	/* magic number */
	write( outfd, &magic, sizeof(magic));

	/* number of files */
	write( outfd, &nfiles, sizeof(nfiles));

	close( outfd );

	exit(EXIT_SUCCESS);

usage:
	printf(USAGE);
failure:
	exit(EXIT_FAILURE);
}


/*
 * gets the file's size
 */
off_t getfilesize( const char* filename )
{
	struct stat file;

	if (!stat( filename, &file ))
		return file.st_size;

	printf( "unable to get filesize of %s\n", filename );
	return 0;
}


/*
 * file exists
 */
int fileexists( const char* filename )
{
	struct stat file;

	if (!stat( filename, &file ))
		return 1;
	
	return 0;
}
