/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_ship.c
 *
 * @brief Handles the ship development routines.
 */

#include "dev_ship.h"

#include "naev.h"

#include "SDL.h"

#include "log.h"
#include "ship.h"
#include "nstring.h"


/**
 * @brief Dumps ship data to csv.
 */
void dship_csv( const char *path )
{
   Ship *s, *s_all;
   int i, n, l;
   SDL_RWops *rw;
   char buf[ 1024 ];

   /* File to output to. */
   rw = SDL_RWFromFile( path, "w" );
   if (rw == NULL) {
      WARN("Unable to open '%s' for writing: %s", path, SDL_GetError());
      return;
   }

   /* Write "header" */
   l = nsnprintf( buf, sizeof(buf),
         "name,class,base_type,price,license,fabricator,"
         "thrust,turn,speed,"
         "crew,mass,cpu,fuel,cap_cargo,"
         "absorb,"
         "armour,armour_regen,"
         "shield,shield_regen,"
         "energy,energy_regen,"
         "slot weapon,slot systems,slot structure\n"
         );
   SDL_RWwrite( rw, buf, l, 1 );

   s_all = ship_getAll( &n );
   for (i=0; i<n; i++) {
      s = &s_all[i];

      l = nsnprintf( buf, sizeof(buf),
            "%s,%s,%s,%"CREDITS_PRI",%s,%s,"
            "%f,%f,%f,"
            "%d,%f,%f,%d,%f,"
            "%f,"
            "%f,%f,"
            "%f,%f,"
            "%f,%f,"
            "%d,%d,%d\n",
            s->name, ship_class(s), s->base_type, s->price, s->license, s->fabricator,
            s->thrust/s->mass, s->turn*180./M_PI, s->speed,
            s->crew, s->mass, s->cpu, s->fuel, s->cap_cargo,
            s->dmg_absorb*100,
            s->armour, s->armour_regen,
            s->shield, s->shield_regen,
            s->energy, s->energy_regen,
            s->outfit_nweapon, s->outfit_nutility, s->outfit_nstructure
            );
      SDL_RWwrite( rw, buf, l, 1 );
   }

   /* Close file. */
   SDL_RWclose( rw );
}

