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

/*
 * Targeting
 */
void RotateToNormal ( double* x_, double* y_, double nx_,double ny_ );
double LinearTrajectoryAngle ( double x_,double y_, double vx_,double vy_, double speed_ );
double AngularTrajectoryAngle ( const Vector2d* pos_, const Vector2d* vel_, const Solid* target_, double speed_ );

#endif /* WEAPON_H */

