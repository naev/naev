/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "outfit.h"
#include "physics.h"
#include "pilot.h"
#include "target.h"

/**
 * @enum WeaponLayer
 * @brief Designates the layer the weapon is on.
 * Automatically set up on creation (player is front, rest is back).
 * Only really matters for rendering order.
 */
typedef enum {
   WEAPON_LAYER_BG, /**< Background layer, behind the pilots. */
   WEAPON_LAYER_FG, /**< Foreground layer, infront of pilots, behind player. */
} WeaponLayer;

/* Weapon status */
typedef enum WeaponStatus_ {
   WEAPON_STATUS_LOCKING,       /**< Weapon is locking on. */
   WEAPON_STATUS_OK,            /**< Weapon is fine */
   WEAPON_STATUS_UNJAMMED,      /**< Survived jamming */
   WEAPON_STATUS_JAMMED,        /**< Got jammed */
   WEAPON_STATUS_JAMMED_SLOWED, /**< Jammed and is now slower. */
} WeaponStatus;

/* Weapon flags. */
#define WEAPON_FLAG_DESTROYED ( 1 << 0 ) /**< Is awaiting clean up. */
#define WEAPON_FLAG_HITTABLE ( 1 << 1 )  /**< Can be hit by stuff. */
#define WEAPON_FLAG_ONLYHITTARGET                                              \
   ( 1 << 2 ) /**< Can only hit target pilot (or asteroids). */
#define WEAPON_FLAG_AIM                                                        \
   ( 1 << 3 ) /**< Weapon should aim and not follow the mouse (only used for   \
                 beams atm). */
#define weapon_isFlag( w, f ) ( ( w )->flags & ( f ) )
#define weapon_setFlag( w, f ) ( ( w )->flags |= ( f ) )
#define weapon_rmFlag( w, f ) ( ( w )->flags &= ~( f ) )

/**
 * @struct Weapon
 *
 * @brief In-game representation of a weapon.
 */
typedef struct Weapon_ {
   WeaponLayer  layer; /**< Weapon layer. */
   unsigned int flags; /**< Weapon flags. */
   Solid        solid; /**< Actually has its own solid :) */
   unsigned int id;    /**< Unique weapon id. */

   int           faction; /**< faction of pilot that shot it */
   unsigned int  parent;  /**< pilot that shot it */
   Target        target;  /**< Weapon target. */
   const Outfit *outfit;  /**< related outfit that fired it or whatnot */

   double  real_vel;       /**< Keeps track of the real velocity. */
   double  dam_mod;        /**< Damage modifier. */
   double  dam_as_dis_mod; /**< Damage as disable modifier. */
   int     voice;          /**< Weapon's voice. */
   double  timer2; /**< Explosion timer for beams, and lockon for ammo. */
   double  paramf; /**< Arbitrary parameter for outfits. */
   double  life;   /**< Total life. */
   double  timer;  /**< mainly used to see when the weapon was fired */
   double  anim;   /**< Used for beam weapon graphics and others. */
   GLfloat r;      /**< Unique random value . */
   int     sprite; /**< Used for spinning outfits. */
   PilotOutfitSlot *mount;   /**< Used for beam weapons. */
   int              lua_mem; /**< Mem table, in case of a Pilot Outfit. */
   double falloff;       /**< Point at which damage falls off. Used to determine
                            slowdown for smart seekers.  */
   double      strength; /**< Calculated with falloff. */
   double      strength_base; /**< Base strength, set via Lua. */
   int         sx;            /**< Current X sprite to use. */
   int         sy;            /**< Current Y sprite to use. */
   Trail_spfx *trail;         /**< Trail graphic if applicable, else NULL. */

   double armour; /**< Health status of the weapon. */

   void ( *think )( struct Weapon_ *, double ); /**< for the smart missiles */

   WeaponStatus status; /**< Weapon status - to check for jamming */
} Weapon;

Weapon *weapon_getStack( void );
Weapon *weapon_getID( unsigned int id );

/* Addition. */
Weapon *weapon_add( PilotOutfitSlot *po, const Outfit *ref, double dir,
                    const vec2 *pos, const vec2 *vel, const Pilot *parent,
                    const Target *target, double time, int aim );

/* targetting. */
int    weapon_inArc( const Outfit *o, const Pilot *parent, const Target *target,
                     const vec2 *pos, const vec2 *vel, double dir, double time );
double weapon_targetFlyTime( const Outfit *o, const Pilot *p, const Target *t );

/* Beam weapons. */
unsigned int beam_start( PilotOutfitSlot *po, double dir, const vec2 *pos,
                         const vec2 *vel, const Pilot *parent,
                         const Target *target, int aim );
void         beam_end( unsigned int beam );

/* Misc stuff. */
void           weapon_hitAI( Pilot *p, const Pilot *shooter, double dmg );
const IntList *weapon_collideQuery( int x1, int y1, int x2, int y2 );
void weapon_collideQueryIL( IntList *il, int x1, int y1, int x2, int y2 );

/* Update. */
void weapons_updatePurge( void );
void weapons_updateCollide( double dt );
void weapons_update( double dt );
void weapons_render( const WeaponLayer layer, double dt );

/* Clean. */
void weapon_init( void );
void weapon_newSystem( void );
void weapon_clear( void );
void weapon_exit( void );
