

#ifndef WEAPON_H
#  define WEAPON__H


#include "outfit.h"
#include "physics.h"

typedef enum { WEAPON_LAYER_BG, WEAPON_LAYER_FG } WeaponLayer;

void weapon_add( const Outfit* outfit, const double dir,
		const Vector2d* pos, const Vector2d* vel, WeaponLayer layer );

void weapons_update( const double dt, WeaponLayer layer );


#endif /* WEAPON_H */
