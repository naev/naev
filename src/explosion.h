/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef EXPLOSION_H
#  define EXPLOSION_H


#include "outfit.h"
#include "pilot.h"


#define EXPL_MODE_SHIP     (1<<0) /**< Affects ships. */
#define EXPL_MODE_MISSILE  (1<<1) /**< Affects missiles. */
#define EXPL_MODE_BOLT     (1<<2) /**< Affects bolts. */


void expl_explode( double x, double y, double vx, double vy,
      double radius, const Damage *dmg,
      const Pilot *parent, int mode );
void expl_explodeDamage( double x, double y, double radius,
      const Damage *dmg, const Pilot *parent, int mode );


#endif /* EXPLOSION_H */

