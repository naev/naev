/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_outfit.c
 *
 * @brief Handles the outfit development routines.
 */

#include "dev_outfit.h"

#include "naev.h"

#include "SDL.h"

#include "log.h"
#include "outfit.h"
#include "damagetype.h"
#include "nstring.h"


/**
 * @brief Dumps the bolt weapon data to csv.
 */
void dout_csvBolt( const char *path )
{
   Outfit *o, *o_all;
   int i, n, l;
   SDL_RWops *rw;
   char buf[ 1024 ];
   Damage *dmg;

   /* File to output to. */
   rw = SDL_RWFromFile( path, "w" );
   if (rw == NULL) {
      WARN("Unable to open '%s' for writing: %s", path, SDL_GetError());
      return;
   }

   /* Write "header" */
   l = nsnprintf( buf, sizeof(buf),
         "name,type,slot,license,"
         "mass,price,"
         "delay,speed,range,falloff,"
         "lockon,energy,heatup,cpu,"
         "track,swivel,"
         "penetrate,dtype,damage,disable\n"
         );
   SDL_RWwrite( rw, buf, l, 1 );

   o_all = outfit_getAll( &n );
   for (i=0; i<n; i++) {
      o = &o_all[i];

      /* Only handle bolt weapons. */
      if (!outfit_isBolt(o))
         continue;

      dmg = &o->u.blt.dmg;
      l = nsnprintf( buf, sizeof(buf),
            "%s,%s,%s,%s,"
            "%f,%"CREDITS_PRI","
            "%f,%f,%f,%f,"
            "%f,%f,%f,%f,"
            "%f,%f,"
            "%f,%s,%f,%f\n",
            o->name, outfit_getType(o), outfit_slotName(o), o->license,
            o->mass, o->price,
            o->u.blt.delay, o->u.blt.speed, o->u.blt.range, o->u.blt.falloff,
            o->u.blt.ew_lockon, o->u.blt.energy, o->u.blt.heatup, o->u.blt.cpu,
            o->u.blt.track, o->u.blt.swivel * 180. / M_PI,
            dmg->penetration*100, dtype_damageTypeToStr(dmg->type), dmg->damage, dmg->disable
            );
      SDL_RWwrite( rw, buf, l, 1 );
   }

   /* Close file. */
   SDL_RWclose( rw );
}

/**
 * @brief Dumps the modification data to csv.
 *
 * Note that this function is primarily intended for balancing core outfits.
 *
 * To that end, it omits a number of seldom-used properties, and is primarily
 * concerned with those that are common to most types of modifications, or at
 * least one category of core outfit.
 */
void dout_csvMod( const char *path )
{
   Outfit *o, *o_all;
   int i, n, l;
   SDL_RWops *rw;
   char buf[ 1024 ];
   ShipStats base, stats;

   /* File to output to. */
   rw = SDL_RWFromFile( path, "w" );
   if (rw == NULL) {
      WARN("Unable to open '%s' for writing: %s", path, SDL_GetError());
      return;
   }

   /* Write "header" */
   l = nsnprintf( buf, sizeof(buf),
         "name,type,slot,size,"
         "license,mass,price,"
         "thrust,turn,speed,fuel,energy_usage,"
         "armour,armour_regen,"
         "shield,shield_regen,"
         "energy,energy_regen,"
         "absorb,cargo,ew_hide\n"
         );
   SDL_RWwrite( rw, buf, l, 1 );

   ss_statsInit( &base );

   o_all = outfit_getAll( &n );
   for (i=0; i<n; i++) {
      o = &o_all[i];

      /* Only handle modifications. */
      if (!outfit_isMod(o))
         continue;

      memcpy( &stats, &base, sizeof(ShipStats) );
      ss_statsModFromList( &stats, o->u.mod.stats, NULL );

      l = nsnprintf( buf, sizeof(buf),
            "%s,%s,%s,%s,"
            "%s,%f,%"CREDITS_PRI","
            "%f,%f,%f,%f,%f,"
            "%f,%f,"
            "%f,%f,"
            "%f,%f,"
            "%f,%f,%f\n",
            o->name, outfit_getType(o), outfit_slotName(o), outfit_slotSize(o),
            o->license, o->mass, o->price,
            o->u.mod.thrust, o->u.mod.turn * 180. / M_PI, o->u.mod.speed, o->u.mod.fuel, stats.energy_usage,
            o->u.mod.armour, o->u.mod.armour_regen,
            o->u.mod.shield, o->u.mod.shield_regen,
            o->u.mod.energy, o->u.mod.energy_regen,
            o->u.mod.absorb,o->u.mod.cargo, stats.ew_hide - 1.
            );
      SDL_RWwrite( rw, buf, l, 1 );
   }

   /* Close file. */
   SDL_RWclose( rw );
}
