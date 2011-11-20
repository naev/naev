/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WEAPON_H
#  define WEAPON__H


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
 * addition
 */
void weapon_add( const Outfit* outfit, const double T,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const Pilot *parent, const unsigned int target );


/*
 * Beam weapons.
 */
unsigned int beam_start( const Outfit* outfit,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const Pilot *parent, const unsigned int target,
      const PilotOutfitSlot *mount );
void beam_end( const unsigned int parent, unsigned int beam );


/*
 * Misc stuff.
 */
void weapon_explode( double x, double y, double radius,
      int dtype, double damage,
      const Pilot *parent, int mode );


/*
 * update
 */
void weapons_update( const double dt );
void weapons_render( const WeaponLayer layer, const double dt );


/*
 * clean
 */
void weapon_clear (void);
void weapon_exit (void);

// Targeting

// LinearTrajectoryAngle finds the angle to hit a target using a relative position, velocity and projectile speed_
//
// x,y = relative position (source-target)
// vx,vy = relative velocity (target-source)
// speed_ to find angle for
// returns -PI to +PI or 1000. on fail
double LinearTrajectoryAngle ( double x_,double y_, double vx_,double vy_, double speed_ );
//AngularTrajectoryAngle
//uses the velocity diffrence in the targets solid to calc a circular path and return a collision angle for it at speed
//Probably should be static but may be wanted by the AI for aimming
double AngularTrajectoryAngle ( Solid* source_, Solid* target_, double speed_ );

#endif /* WEAPON_H */

