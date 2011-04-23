
#include <stdlib.h> /* exit() */
#include <stdio.h> /* printf() */
#include <stdint.h> /* uint32_t */
#include <unistd.h> /* getopt */
#include <getopt.h> /* getopt_long */


#include "pack.h"


#define COMMAND_NORMAL        0
#define COMMAND_DECOMPRESS    1


static void print_usage( char* appname )
{
   printf(
         "Usage is: %s [options] file [inputs]\n"
         "   If no inputs are specified, it lists the contents of file.\n"
         "   Options:\n"
         /*
         "     -d       Decompress file instead of compressing (will overwrite stuff).\n"
         */
         , appname );
}


int main( int argc, char** argv )
{
   static struct option long_options[] = {
      { "help", no_argument, 0, 'h' },
      { "decompress", no_argument, 0, 'd' },
      { NULL, 0, 0, 0 }
   };
   int option_index;
   char* packfile;
   char** list;
   uint32_t nlist;
   uint32_t nfiles;
   uint32_t i;
   /*int cmd;*/
   int c;

   /* Enforce at least one parameter. */
   if (argc == 1) {
      fprintf(stderr, "Missing file\n");
      goto usage;
   }

   /* Handle parameters. */
   /*cmd = COMMAND_NORMAL;*/
   while ((c = getopt_long( argc, argv,
         "hd",
         long_options, &option_index)) != -1) {
      switch (c) {
         case 'h':
            goto usage;
            break;
/*
         case 'd':
            cmd = COMMAND_DECOMPRESS;
            break;
*/
      }
   }

   packfile = argv[1];
   nfiles = (uint32_t)argc - 2;
   argv+=2;

   if (nfiles == 0) { /* no files, list what it has */
      list = pack_listfiles( packfile, &nlist );
      fprintf(stdout, "Packfile '%s' contains:\n", packfile);
      for (i=0; i<nlist; i++) {
         fprintf(stdout, "   %04d   %s\n", i, list[i]);
         free(list[i]);
      }
      free(list);
   }
   else { /* create a packfile */
      pack_files( packfile, (const char**)argv, nfiles );
   }
   exit(EXIT_SUCCESS);

usage:
   print_usage( argv[0] );
   exit(EXIT_SUCCESS);
}


