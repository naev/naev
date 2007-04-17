
#include <stdlib.h> /* exit() */
#include <stdio.h> /* printf() */

#include "pack.h"


#define USAGE  "Usage is: %s output inputs\n",argv[0]


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

	pack_files( outfile, (const char**)argv, nfiles );
	exit(EXIT_SUCCESS);

usage:
	printf(USAGE);
	exit(EXIT_SUCCESS);
}

