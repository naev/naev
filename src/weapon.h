/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "outfit.h"
#include "physics.h"
#include "pilot.h"

/**
 * @enum WeaponLayer
 * @brief Designates the layer the weapon is on.
 * Automatically set up on creation (player is front, rest is back).
 */
typedef enum { WEAPON_LAYER_BG, WEAPON_LAYER_FG } WeaponLayer;

/*
 * Addition.
 */
void weapon_add( PilotOutfitSlot *po, const double T,
      const double dir, const vec2* pos, const vec2* vel,
      const Pilot *parent, const unsigned int target, double time, int aim );

/*
 * Beam weapons.
 */
unsigned int beam_start( PilotOutfitSlot *po,
      const double dir, const vec2* pos, const vec2* vel,
      const Pilot *parent, const unsigned int target, int aim );
void beam_end( const unsigned int parent, unsigned int beam );

/*
 * Misc stuff.
 */
void weapon_explode( double x, double y, double radius,
      int dtype, double damage,
      const Pilot *parent, int mode );
void weapon_hitAI( Pilot *p, const Pilot *shooter, double dmg );

/*
 * Update.
 */
void weapons_updatePurge (void);
void weapons_updateCollide( double dt );
void weapons_update( double dt );
void weapons_render( const WeaponLayer layer, const double dt );

/*
 * Clean.
 */
void weapon_init (void);
void weapon_clear (void);
void weapon_exit (void);
