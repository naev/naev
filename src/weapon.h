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

#define weapon_isSmart(w)     (w->think != NULL) /**< Checks if the weapon w is smart. */

/* Weapon status */
typedef enum WeaponStatus_ {
   WEAPON_STATUS_LOCKING,  /**< Weapon is locking on. */
   WEAPON_STATUS_OK,       /**< Weapon is fine */
   WEAPON_STATUS_UNJAMMED, /**< Survived jamming */
   WEAPON_STATUS_JAMMED,   /**< Got jammed */
   WEAPON_STATUS_JAMMED_SLOWED, /**< Jammed and is now slower. */
} WeaponStatus;

/* Weapon flags. */
#define WEAPON_FLAG_DESTROYED       (1<<0) /* Is awaiting clean up. */
#define WEAPON_FLAG_HITTABLE        (1<<1) /* Can be hit by stuff. */
#define weapon_isFlag(w,f)    ((w)->flags & (f))
#define weapon_setFlag(w,f)   ((w)->flags |= (f))
#define weapon_rmFlag(w,f)    ((w)->flags &= ~(f))

/**
 * @struct Weapon
 *
 * @brief In-game representation of a weapon.
 */
typedef struct Weapon_ {
   WeaponLayer layer;   /**< Weapon layer. */
   unsigned int flags;  /**< Weapon flags. */
   Solid solid;         /**< Actually has its own solid :) */
   unsigned int id;     /**< Unique weapon id. */

   int faction;         /**< faction of pilot that shot it */
   unsigned int parent; /**< pilot that shot it */
   unsigned int target; /**< target to hit, only used by seeking things */
   const Outfit* outfit; /**< related outfit that fired it or whatnot */

   double real_vel;     /**< Keeps track of the real velocity. */
   double dam_mod;      /**< Damage modifier. */
   double dam_as_dis_mod; /**< Damage as disable modifier. */
   int voice;           /**< Weapon's voice. */
   double timer2;       /**< Explosion timer for beams, and lockon for ammo. */
   double paramf;       /**< Arbitrary parameter for outfits. */
   double life;         /**< Total life. */
   double timer;        /**< mainly used to see when the weapon was fired */
   double anim;         /**< Used for beam weapon graphics and others. */
   GLfloat r;           /**< Unique random value . */
   int sprite;          /**< Used for spinning outfits. */
   PilotOutfitSlot *mount; /**< Used for beam weapons. */
   int lua_mem;         /**< Mem table, in case of a Pilot Outfit. */
   double falloff;      /**< Point at which damage falls off. Used to determine slowdown for smart seekers.  */
   double strength;     /**< Calculated with falloff. */
   int sx;              /**< Current X sprite to use. */
   int sy;              /**< Current Y sprite to use. */
   Trail_spfx *trail;   /**< Trail graphic if applicable, else NULL. */

   double armour;       /**< Health status of the weapon. */

   void (*think)(struct Weapon_*, double); /**< for the smart missiles */

   WeaponStatus status; /**< Weapon status - to check for jamming */
} Weapon;

/*
 * Addition.
 */
void weapon_add( PilotOutfitSlot *po, const Outfit *ref, double T,
      double dir, const vec2* pos, const vec2* vel,
      const Pilot *parent, const unsigned int target, double time, int aim );

/*
 * Beam weapons.
 */
unsigned int beam_start( PilotOutfitSlot *po,
      double dir, const vec2* pos, const vec2* vel,
      const Pilot *parent, const unsigned int target, int aim );
void beam_end( unsigned int beam );

/*
 * Misc stuff.
 */
void weapon_hitAI( Pilot *p, const Pilot *shooter, double dmg );

/*
 * Update.
 */
void weapons_updatePurge (void);
void weapons_updateCollide( double dt );
void weapons_update( double dt );
void weapons_render( const WeaponLayer layer, double dt );

/*
 * Clean.
 */
void weapon_init (void);
void weapon_newSystem (void);
void weapon_clear (void);
void weapon_exit (void);
