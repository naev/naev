/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_outfit.c
 *
 * @brief Handles the outfit developement routines.
 */

#include "dev_outfit.h"

#include "naev.h"

#include "SDL.h"

#include "outfit.h"


/**
 * @brief Dumps the bolt weapon data to csv.
 */
void dout_csvBolt( const char *path )
{
   Outfit *o;
   int i, n, l;
   SDL_RWops *rw;
   char buf[ 1024 ];

   /* File to output to. */
   rw = SDL_RWFromFile( path, "w" );

   /* Write "header" */
   l = snprintf( buf, sizeof(buf),
         "name,type,slot,license,mass,price,delay,speed,range,falloff,"
         "accuracy,lockon,energy,cpu,dtype,damage,dps,eps\n"
         );
   SDL_RWwrite( rw, buf, l, 1 );

   o = outfit_getAll( &n );
   for (i=0; i<n; i++) {

      /* Only handle bolt weapons. */
      if (!outfit_isBolt(o))
         continue;
  
      l = snprintf( buf, sizeof(buf),
            "name,type,slot,license,mass,price,delay,speed,range,falloff,"
            "accuracy,lockon,energy,cpu,dtype,damage,dps,eps\n"
            );
      SDL_RWwrite( rw, buf, l, 1 );
   }

   /* Close file. */
   SDL_RWclose( rw );
}


