/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file explosion.c
 *
 * @brief Handles gigantic explosions.
 */


#include "explosion.h"

#include "naev.h"

#include "log.h"
#include "pilot.h"
#include "weapon.h"
#include "spfx.h"
#include "rng.h"


static int exp_s = -1; /**< Small explosion spfx. */
static int exp_m = -1; /**< Medium explosion spfx. */
static int exp_l = -1; /**< Large explosion spfx. */


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
void expl_explode( double x, double y, double vx, double vy,
      double radius, const Damage *dmg,
      const Pilot *parent, int mode )
{
   int i, n;
   double a, d;
   double area;
   double ex, ey;
   int layer;
   int efx;

   /* Standard stuff - lazy allocation. */
   if (exp_s == -1) {
      exp_s = spfx_get("ExpS");
      exp_m = spfx_get("ExpM");
      exp_l = spfx_get("ExpL");
   }
   layer = SPFX_LAYER_FRONT;

   /* Number of explosions. */
   area = M_PI * pow2(radius);
   n = (int)(area / 100.);

   /* Create explosions. */
   for (i=0; i<n; i++) {
      /* Get position. */
      a = RNGF()*360.;
      d = RNGF()*(radius-5.) + 5.;
      ex = d*cos(a);
      ey = d*sin(a);

      /* Create explosion. */
      efx = (RNG(0,2)==0) ? exp_m : exp_s;
      spfx_add( efx, x+ex, y+ey, vx, vy, layer );
   }

   /* Final explosion. */
   spfx_add( exp_l, x, y, vx, vy, layer );

   /* Run the damage. */
   if (dmg != NULL)
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
void expl_explodeDamage( double x, double y, double radius,
      const Damage *dmg, const Pilot *parent, int mode )
{
   /* Explosion affects ships. */
   if (mode & EXPL_MODE_SHIP)
      pilot_explode( x, y, radius, dmg, parent );

   /* Explosion affects missiles and bolts. */
   if ((mode & EXPL_MODE_MISSILE) || (mode & EXPL_MODE_BOLT))
      weapon_explode( x, y, radius, dmg->type, dmg->damage, parent, mode );
}

