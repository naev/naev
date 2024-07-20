#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "SDL.h"

int nfile_simplifyPath( char path[static 1] )
{
   char **dirnames = array_create( char * );
   char  *saveptr  = NULL;
   size_t n        = strlen( path );
   int absolute = (path[0]=='/');
   const char  *token    = SDL_strtokr( path, "/", &saveptr );

   while ( token != NULL ) {
      /* Skip noop. */
      if ( ( strcmp( token, "" ) == 0 ) || ( strcmp( token, "." ) == 0 ) ) {
         token = SDL_strtokr( NULL, "/", &saveptr );
         continue;
      }

      /* Go up if ".." */
      if ( strcmp( token, ".." ) == 0 ) {
         int dn = array_size( dirnames );
         if ( dn > 0 ) {
            free( dirnames[dn - 1] );
            array_erase( &dirnames, &dirnames[dn - 1], &dirnames[dn] );
         }
      } else {
         array_push_back( &dirnames, strdup( token ) );
      }

      /* On to the next one. */
      token = SDL_strtokr( NULL, "/", &saveptr );
   }

   /* If nothing, we're empty. */
   if ( array_size( dirnames ) <= 0 ) {
      array_free( dirnames );
      path[0] = '\0';
      return 0;
   }

   /* Build back the path, assume 'path' is smaller than simplified one. */
   size_t s = 0;
   for ( int i = 0; i < array_size( dirnames ); i++ ) {
      char *ds = dirnames[i];
      if ((absolute || ( s > 0 )) && ( s < n ) )
         path[s++] = '/';
      for ( size_t j = 0; j < strlen( ds ); j++ )
         if ( s < n )
            path[s++] = ds[j];
      free( ds );
   }
   path[s] = '\0';
   array_free( dirnames );
   return 0;
}
