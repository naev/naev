
#include <stdlib.h> /* exit() */
#include <stdio.h> /* printf() */

#include "pack.h"


#define USAGE  "Usage is: %s output input\n",argv[0]


int main( int argc, char** argv )
{  
	switch (argc) {
		case 1:
			fprintf(stderr, "Missing output file\n");
		case 2:
			fprintf(stderr, "Missing input file/s\n");
			goto usage;
			break;
	}

	char* outfile = argv[1];
	uint32_t nfiles = (uint32_t)argc - 2;
	argv+=2;

	printf("%d\n",pack_check(outfile));
	pack_files( outfile, argv, nfiles );

	Packfile packfile;

	printf("%s\n", argv[0]);
	pack_open( &packfile, outfile, argv[2] );

	char* buf = calloc(100,1);
	nfiles = pack_read( &packfile, buf, 100 );
	printf("%d -> <%s>\n", nfiles, buf);
	free(buf);

	exit(EXIT_SUCCESS);

usage:
	printf(USAGE);
	exit(EXIT_SUCCESS);
}

