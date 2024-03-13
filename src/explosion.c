/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file explosion.c
 *
 * @brief Handles gigantic explosions.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "explosion.h"

#include "log.h"
#include "pilot.h"
#include "rng.h"
#include "spfx.h"
#include "weapon.h"

static int exp_s   = -1; /**< Small explosion spfx. */
static int exp_m   = -1; /**< Medium explosion spfx. */
static int exp_l   = -1; /**< Large explosion spfx. */
static int exp_200 = -1; /**< 200 radius explsion spfx. */
static int exp_300 = -1; /**< 300 radius explosion spfx. */
static int exp_400 = -1; /**< 400 radius explosion spfx. */
static int exp_500 = -1; /**< 500 radius explosion spfx. */
static int exp_600 = -1; /**< 600 radius explosion spfx. */

/**
 * @brief Does explosion in a radius (damage and graphics).
 *
 *    @param x X position of explosion center.
 *    @param y Y position of explosion center.
 *    @param vx X velocity of explosion center.
 *    @param vy Y velocity of explosion center.
 *    @param radius Radius of the explosion.
 *    @param dmg Damage characteristics.
 *    @param parent Parent of the explosion, NULL is none.
 *    @param mode Defines the explosion behaviour.
 */
void expl_explode( double x, double y, double vx, double vy, double radius,
                   const Damage *dmg, const Pilot *parent, int mode )
{
   int layer;
   int efx;

   /* Standard stuff - lazy allocation. */
   if ( exp_s == -1 ) {
      /* TODO This is all horrible and I wish we could either parametrize it or
       * get rid of the hardcoding. */
      exp_s   = spfx_get( "ExpS" );
      exp_m   = spfx_get( "ExpM" );
      exp_l   = spfx_get( "ExpL" );
      exp_200 = spfx_get( "Exp200" );
      exp_300 = spfx_get( "Exp300" );
      exp_400 = spfx_get( "Exp400" );
      exp_500 = spfx_get( "Exp500" );
      exp_600 = spfx_get( "Exp600" );
   }
   layer = SPFX_LAYER_FRONT;

   if ( radius < 40. / 2. )
      efx = exp_s;
   else if ( radius < 70. / 2. )
      efx = exp_m;
   else if ( radius < 100. / 2. )
      efx = exp_l;
   else if ( radius < 200. / 2. )
      efx = exp_200;
   else if ( radius < 300. / 2. )
      efx = exp_300;
   else if ( radius < 400. / 2. )
      efx = exp_400;
   else if ( radius < 500. / 2. )
      efx = exp_500;
   // else if (radius < 600./2.)
   else
      efx = exp_600;

   /* Final explosion. */
   spfx_add( efx, x, y, vx, vy, layer );

   /* Run the damage. */
   if ( dmg != NULL )
      expl_explodeDamage( x, y, radius, dmg, parent, mode );
}

/**
 * @brief Does explosion damage in a radius.
 *
 *    @param x X position of explosion center.
 *    @param y Y position of explosion center.
 *    @param radius Radius of the explosion.
 *    @param dmg Damage characteristics.
 *    @param parent Parent of the explosion, 0 is none.
 *    @param mode Defines the explosion behaviour.
 */
void expl_explodeDamage( double x, double y, double radius, const Damage *dmg,
                         const Pilot *parent, int mode )
{
   /* Explosion affects ships. */
   if ( mode & EXPL_MODE_SHIP )
      pilot_explode( x, y, radius, dmg, parent );

#if 0
   /* Explosion affects missiles and bolts. */
   if ((mode & EXPL_MODE_MISSILE) || (mode & EXPL_MODE_BOLT))
      weapon_explode( x, y, radius, dmg->type, dmg->damage, parent, mode );
#endif
}
