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
   Outfit *o, *o_all;
   int i, n, l;
   SDL_RWops *rw;
   char buf[ 1024 ];
   double shots, dps, eps;

   /* File to output to. */
   rw = SDL_RWFromFile( path, "w" );

   /* Write "header" */
   l = snprintf( buf, sizeof(buf),
         "name,type,slot,license,mass,price,delay,speed,range,falloff,"
         "accuracy,lockon,energy,cpu,dtype,damage,shots,dps,eps\n"
         );
   SDL_RWwrite( rw, buf, l, 1 );

   o_all = outfit_getAll( &n );
   for (i=0; i<n; i++) {
      o = &o_all[i];

      /* Only handle bolt weapons. */
      if (!outfit_isBolt(o))
         continue;
 
      shots = 1. / o->u.blt.delay;
      dps  += shots * o->u.blt.damage;
      eps  += shots * o->u.blt.energy;

      l = snprintf( buf, sizeof(buf),
            "%s,%s,%s,%s,"
            "%f,%d,"
            "%f,%f,%f,%f,"
            "%f,%f,%f,%f,"
            "%s,%f,"
            "%f,%f,%f\n",
            o->name, outfit_getType(o), outfit_slotName(o), o->license,
            o->mass, o->price,
            o->u.blt.delay, o->u.blt.speed, o->u.blt.range, o->u.blt.falloff,
            o->u.blt.accuracy, o->u.blt.ew_lockon, o->u.blt.energy, o->u.blt.cpu,
            outfit_damageTypeToStr(o->u.blt.dtype), o->u.blt.damage, 
            shots, dps, eps
            );
      SDL_RWwrite( rw, buf, l, 1 );
   }

   /* Close file. */
   SDL_RWclose( rw );
}


