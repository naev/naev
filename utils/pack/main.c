
#include <stdlib.h> /* exit() */
#include <stdio.h> /* printf() */
#include <stdint.h> /* uint32_t */

#include "pack.h"


#define USAGE  "Usage is: %s output inputs\n",argv[0]


int main( int argc, char** argv )
{
   char* packfile;
   char** list;
   uint32_t nlist;
   int i;
   uint32_t nfiles;

   if (argc == 1) {
      fprintf(stderr, "Missing output file\n");
      goto usage;
   }

   packfile = argv[1];
   nfiles = (uint32_t)argc - 2;
   argv+=2;

   if (nfiles == 0) { /* no files, list what it has */
      list = pack_listfiles( packfile, &nlist );
      fprintf(stdout, "Packfile '%s' contains:\n", packfile);
      for (i=0; i<nlist; i++) {
         fprintf(stdout, "   %03d   %s\n", i, list[i]);
         free(list[i]);
      }
      free(list);
   }
   else { /* create a packfile */
      pack_files( packfile, (const char**)argv, nfiles );
   }
   exit(EXIT_SUCCESS);

usage:
   printf(USAGE);
   exit(EXIT_SUCCESS);
}

