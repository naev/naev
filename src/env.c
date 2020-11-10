/*
 * See Licensing and Copyright notice in naev.h
 */


#include "env.h"

#include <assert.h>
#include <stdlib.h>


env_t env;


void env_detect( int argc, char **argv )
{
   (void) argc;
   static short once = 0;
   assert( once == 0 );
   once = 1;

   env.appimage = getenv( "APPIMAGE" );
   if ( env.appimage != NULL ) {
      env.isAppImage = 1;
      env.argv0      = getenv( "ARGV0" );
      env.appdir     = getenv( "APPDIR" );
   }
   else {
      env.isAppImage = 0;
      env.argv0      = argv[0];
   }
}
