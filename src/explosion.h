/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef EXPLOSION_H
#  define EXPLOSION__H


#include "outfit.h"
#include "pilot.h"


#define EXPL_MODE_SHIP     (1<<0) /**< Affects ships. */
#define EXPL_MODE_MISSILE  (1<<1) /**< Affects missiles. */
#define EXPL_MODE_BOLT     (1<<2) /**< Affects bolts. */


void expl_explode( double x, double y, double vx, double vy,
      double radius, DamageType dtype, double damage, double penetration,
      const Pilot *parent, int mode );
void expl_explodeDamage( double x, double y, double radius,
      DamageType dtype, double damage, double penetration,
      const Pilot *parent, int mode );


#endif /* EXPLOSION_H */

