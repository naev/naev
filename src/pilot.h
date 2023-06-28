/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot_flags.h"

#include "ai.h"
#include "commodity.h"
#include "effect.h"
#include "faction.h"
#include "ntime.h"
#include "outfit.h"
#include "physics.h"
#include "ship.h"
#include "sound.h"
#include "space.h"
#include "spfx.h"
#include "lvar.h"

#define PLAYER_ID       1 /**< Player pilot ID. */

/* Hyperspace parameters. */
#define HYPERSPACE_ENGINE_DELAY  3. /**< Time to warm up engine (seconds). */
#define HYPERSPACE_FLY_DELAY     5. /**< Time it takes to hyperspace (seconds). */
#define HYPERSPACE_DUST_BLUR     3. /**< How long it takes for space dust to start blurring (seconds). */
#define HYPERSPACE_DUST_LENGTH   250 /**< Length the space dust blur to at max (pixels). */
#define HYPERSPACE_FADEOUT       1. /**< How long the fade is (seconds). */
#define HYPERSPACE_FADEIN        1. /**< How long the fade is (seconds). */
#define HYPERSPACE_THRUST        2000./**< How much thrust you use in hyperspace. */
#define HYPERSPACE_VEL           (2.*HYPERSPACE_THRUST*HYPERSPACE_FLY_DELAY) /**< Velocity at hyperspace. */
#define HYPERSPACE_ENTER_MIN     (HYPERSPACE_VEL*0.3) /**< Minimum entering distance. */
#define HYPERSPACE_ENTER_MAX     (HYPERSPACE_VEL*0.4) /**< Maximum entering distance. */
#define HYPERSPACE_EXIT_MIN      1500. /**< Minimum distance to begin jumping. */
/* Land/takeoff. */
#define PILOT_LANDING_DELAY      1. /**< Delay for land animation. */
#define PILOT_TAKEOFF_DELAY      1. /**< Delay for takeoff animation. */
/* Refueling. */
#define PILOT_REFUEL_TIME        3. /**< Time to complete refueling. */
#define PILOT_REFUEL_QUANTITY    100 /**< Amount transferred per refuel. */
/* Misc. */
#define PILOT_SIZE_APPROX        0.8   /**< approximation for pilot size */
#define PILOT_WEAPON_SETS        10    /**< Number of weapon sets the pilot has. */
#define PILOT_WEAPSET_MAX_LEVELS 2     /**< Maximum amount of weapon levels. */
#define PILOT_REVERSE_THRUST     0.4   /**< Ratio of normal thrust to apply when reversing. */
#define PILOT_PLAYER_NONTARGETABLE_TAKEOFF_DELAY 5. /**< Time the player is safe (from being targetted) after takeoff. */
#define PILOT_PLAYER_NONTARGETABLE_JUMPIN_DELAY 5. /**< Time the player is safe (from being targetted) after jumping in. */

/* Pilot-related hooks. */
enum {
   PILOT_HOOK_NONE,      /**< No hook. */
   PILOT_HOOK_CREATION,  /**< Pilot was created. */
   PILOT_HOOK_DEATH,     /**< Pilot died. */
   PILOT_HOOK_BOARDING,  /**< Player is boarding. */
   PILOT_HOOK_BOARD,     /**< Player got boarded. */
   PILOT_HOOK_BOARD_ALL,  /**< Pilot got boarded. */
   PILOT_HOOK_DISABLE,   /**< Pilot got disabled. */
   PILOT_HOOK_UNDISABLE, /**< Pilot recovered from being disabled. */
   PILOT_HOOK_JUMP,      /**< Pilot jumped. */
   PILOT_HOOK_HAIL,      /**< Pilot is hailed. */
   PILOT_HOOK_LAND,      /**< Pilot is landing. */
   PILOT_HOOK_ATTACKED,  /**< Pilot is in manual override and is being attacked. */
   PILOT_HOOK_DISCOVERED,  /**< Pilot is in manual override and is discovered. */
   PILOT_HOOK_SCAN,      /**< Pilot has scanned another pilot. */
   PILOT_HOOK_SCANNED,   /**< Pilot has been scanned by another pilot. */
   PILOT_HOOK_IDLE,      /**< Pilot is in manual override and has just become idle. */
   PILOT_HOOK_EXPLODED,  /**< Pilot died and exploded (about to be removed). */
   PILOT_HOOK_LOCKON,    /**< Pilot had a launcher lockon. */
   PILOT_HOOK_STEALTH,   /**< Pilot either stealthed or destealthed. */
};

/* Damage */
#define PILOT_HOSTILE_THRESHOLD  0.09 /**< Point at which pilot becomes hostile. */
#define PILOT_HOSTILE_DECAY      0.005 /**< Rate at which hostility decays. */

/* Makes life easier */
#define pilot_isPlayer(p)   pilot_isFlag(p,PILOT_PLAYER) /**< Checks if pilot is a player. */
#define pilot_isDisabled(p) pilot_isFlag(p,PILOT_DISABLED) /**< Checks if pilot is disabled. */
#define pilot_isStopped(p)  (VMOD(p->solid.vel) <= MIN_VEL_ERR)
/* We would really have to recursively go up all the parents to check, but we're being cheap. */
#define pilot_isWithPlayer(p) ((p)->faction == FACTION_PLAYER || ((p)->parent == PLAYER_ID))

/**
 * @brief Contains the state of the outfit.
 *
 * Currently only applicable to beam weapons.
 */
typedef enum PilotOutfitState_ {
   PILOT_OUTFIT_OFF,    /**< Normal state. */
   PILOT_OUTFIT_WARMUP, /**< Outfit is starting to warm up. */
   PILOT_OUTFIT_ON,     /**< Outfit is activated and running. */
   PILOT_OUTFIT_COOLDOWN /**< Outfit is cooling down. */
} PilotOutfitState;

/**
 * @brief Stores outfit ammo.
 */
typedef struct PilotOutfitAmmo_ {
   int quantity;        /**< Amount of ammo. */
   int deployed;        /**< For fighter bays. */
   double lockon_timer; /**< Locking on timer. */
   int in_arc;          /**< In arc. */
} PilotOutfitAmmo;

/**
 * @brief Stores an outfit the pilot has.
 */
typedef struct PilotOutfitSlot_ {
   int id;           /**< Position in the global slot list. */

   /* Outfit slot properties. */
   const Outfit* outfit;  /**< Associated outfit. */
   int active;            /**< Slot is an active slot. */
   ShipOutfitSlot *sslot; /**< Ship outfit slot. */

   /* Heat. */
   double heat_T;    /**< Slot temperature. [K] */
   double heat_C;    /**< Slot heat capacity. [W/K] */
   double heat_area; /**< Slot area of contact with ship hull. [m^2] */
   double heat_start;/**< Slot heat at the beginning of a cooldown period. */

   /* Current state. */
   PilotOutfitState state; /**< State of the outfit. */
   double stimer;    /**< State timer, tracking current state. */
   double timer;     /**< Used to store when it was last used. */
   double rtimer;    /**< Used to store when a reload can happen. */
   double progress;  /**< Used to store state progress and used by Lua outfits. */
   int level;        /**< Level in current weapon set (-1 is none). */
   int weapset;      /**< First weapon set that uses the outfit (-1 is none). */
   unsigned int inrange; /**< Should the slot be shut down when not inrange? (only used for beams). */

   /* Type-specific data. */
   union {
      unsigned int beamid; /**< ID of the beam used in this outfit, only used for beams. */
      PilotOutfitAmmo ammo;/**< Ammo for launchers. */
   } u;

   /* In the case of Lua stuff. */
   int lua_mem; /**< Lua reference to the memory table of the specific outfit. */
   ShipStats lua_stats; /**< Intrinsic ship stats for the outfit calculated on the fly. Used only by Lua outfits. */
} PilotOutfitSlot;

/**
 * @brief A pilot Weapon Set Outfit.
 */
typedef struct PilotWeaponSetOutfit_ {
   int level;              /**< Level of trigger. */
   double range2;          /**< Range squared of this specific outfit. */
   int slotid;             /**< ID of the slot associated with the weapon set. */
} PilotWeaponSetOutfit;

/**
 * @brief A weapon set represents a set of weapons that have an action.
 *
 * By default a weapon set indicates what weapons are enabled at a given time.
 *  However they can also be used to launch weapons.
 */
typedef struct PilotWeaponSet_ {
   int type;      /**< Type of the weaponset. */
   int active;    /**< Whether or not it's currently firing. */
   PilotWeaponSetOutfit *slots; /**< Slots involved with the weapon set. */
   /* Only applicable to weapon type. */
   int inrange;   /**< Whether or not to fire only if the target is inrange. */
   int manual;    /**< Whether or not is manually aiming. */
   double range[PILOT_WEAPSET_MAX_LEVELS]; /**< Range of the levels in the outfit slot. */
   double speed[PILOT_WEAPSET_MAX_LEVELS]; /**< Speed of the levels in the outfit slot. */
} PilotWeaponSet;

/**
 * @brief Stores a pilot commodity.
 */
typedef struct PilotCommodity_ {
   const Commodity *commodity;/**< Associated commodity. */
   int quantity;           /**< Amount player has. */
   unsigned int id;        /**< Special mission id for cargo, 0 means none. */
} PilotCommodity;

/**
 * @brief A wrapper for pilot hooks.
 */
typedef struct PilotHook_ {
   int type;         /**< Type of hook. */
   unsigned int id;  /**< Hook ID associated with pilot hook. */
} PilotHook;

/**
 * @brief Different types of escorts.
 */
typedef enum EscortType_e {
   ESCORT_TYPE_NULL,       /**< Invalid escort type. */
   ESCORT_TYPE_BAY,        /**< Escort is from a fighter bay, controllable by it's parent and can dock. */
   ESCORT_TYPE_MERCENARY,  /**< Escort is a mercenary, controllable by it's parent. */
   ESCORT_TYPE_FLEET,      /**< Escort is part of the player's fleet. */
} EscortType_t;

/**
 * @brief Stores an escort.
 */
typedef struct Escort_s {
   char *ship;          /**< Type of the ship escort is flying. */
   EscortType_t type;   /**< Type of escort. */
   unsigned int id;     /**< ID of in-game pilot. */
   /* TODO: something better than this */
   int persist;         /**< True if escort should respawn on takeoff/landing */
} Escort_t;

/**
 * @brief The representation of an in-game pilot.
 */
typedef struct Pilot_ {
   unsigned int id;  /**< pilot's id, used for many functions */
   char* name;       /**< pilot's name (if unique) */

   /* Fleet/faction management. */
   int faction;      /**< Pilot's faction. */
   int presence;     /**< Presence being used by the pilot. */

   /* Object characteristics */
   const Ship* ship; /**< ship pilot is flying */
   Solid solid;      /**< Associated solid (physics) */
   double base_mass; /**< Ship mass plus core outfit mass. */
   double mass_cargo;/**< Amount of cargo mass added. */
   double mass_outfit;/**< Amount of outfit mass added. */
   int tsx;          /**< current sprite x position, calculated on update. */
   int tsy;          /**< current sprite y position, calculated on update. */
   Trail_spfx** trail;/**< Array of pointers to pilot's trails. */

   /* Properties. */
   int cpu;       /**< Amount of CPU the pilot has left. */
   int cpu_max;   /**< Maximum amount of CPU the pilot has. */
   double crew;   /**< Crew amount the player has (display it as (int)floor(), but it's analogue. */
   double cap_cargo;/**< Pilot's cargo capacity. */

   /* Movement */
   double thrust;    /**< Pilot's thrust in px/s^2. */
   double thrust_base;/**< Pilot's base thrust in px/s^2 (not modulated by mass). */
   double speed;     /**< Pilot's speed in px/s. */
   double speed_base;/**< Pilot's base speed in px/s (not modulated by mass). */
   double speed_limit;/**< Pilot's maximum speed in px/s if limited by lua call. */
   double turn;      /**< Pilot's turn in rad/s. */
   double turn_base; /**< Pilot's base turn in rad/s (not modulated by mass). */

   /* Current health */
   double armour;    /**< Current armour. */
   double stress;    /**< Current disable damage level. */
   double shield;    /**< Current shield. */
   double armour_max;/**< Maximum armour. */
   double shield_max;/**< Maximum shield. */
   double armour_regen;/**< Armour regeneration rate (per second). */
   double shield_regen;/**< Shield regeneration rate (per second). */
   double dmg_absorb;/**< Ship damage absorption [0:1] with 1 being 100%. */
   double fuel_max;  /**< Maximum fuel. */
   double fuel;      /**< Current fuel. */
   double fuel_consumption; /**< Fuel consumed per jump. */

   /* Energy is handled a bit differently. */
   double energy;    /**< Current energy. */
   double energy_max; /**< Maximum energy. */
   double energy_regen; /**< Energy regeneration rate (per second). */
   double energy_tau; /**< Tau regeneration rate for energy. */
   double energy_loss; /**< Linear loss that bypasses the actual RC circuit stuff. */

   /* Defensive Electronic Warfare. */
   double ew_detection; /**< Main detection. */
   double ew_evasion;   /**< Evasion. */
   double ew_stealth;   /**< Stealth. */
   /* Defensive Electronic Warfare. */
   double ew_mass;      /**< Mass factor. */
   double ew_asteroid;  /**< Asteroid field factor, affects hide. */
   double ew_jumppoint; /**< Jump point factor, affects stealth. */
   /* misc. */
   double ew_stealth_timer; /**< Stealth timer. */

   /* Heat. */
   double heat_T;    /**< Ship temperature. [K] */
   double heat_C;    /**< Heat capacity of the ship. [W/K] */
   double heat_emis; /**< Ship epsilon parameter (emissivity). [adimensional 0:1] */
   double heat_cond; /**< Ship conductivity parameter. [W/(m*K)] */
   double heat_area; /**< Effective heatsink area of the ship. [m^2] */
   double cdelay;    /**< Duration a full active cooldown takes. */
   double ctimer;    /**< Remaining cooldown time. */
   double heat_start; /**< Temperature at the start of a cooldown. */

   /* Ship statistics. */
   ShipStats ship_stats; /**< Ship stats set by the Lua ship. Only valid if the Lua has ship defined. */
   ShipStats intrinsic_stats; /**< Intrinsic statistics to the ship create on the fly. */
   ShipStats stats;  /**< Pilot's copy of ship statistics, used for comparisons.. */

   /* Ship effects. */
   Effect *effects; /**< Pilot's current activated effects. */

   /* Outfit management */
   PilotOutfitSlot **outfits;        /**< Array (array.h): Pointers to all outfits. */
   PilotOutfitSlot *outfit_structure;/**< Array (array.h): The structure slots. */
   PilotOutfitSlot *outfit_utility;  /**< Array (array.h): The utility slots. */
   PilotOutfitSlot *outfit_weapon;   /**< Array (array.h): The weapon slots. */
   PilotOutfitSlot *outfit_intrinsic;/**< Array (array.h): The intrinsic slots. */

   /* Primarily for AI usage. */
   int ncannons;      /**< Number of cannons equipped. */
   int nturrets;      /**< Number of turrets equipped. */
   int nbeams;        /**< Number of beams equipped. */
   int nfighterbays;  /**< Number of fighter bays available. */
   int nafterburners; /**< Number of afterburners equipped. */
   int outfitlupdate; /**< Has outfits with Lua update scripts. */
   double refuel_amount; /**< Amount to refuel. */

   /* For easier usage. */
   PilotOutfitSlot *afterburner; /**< the afterburner */

   /* Weapon sets. */
   PilotWeaponSet weapon_sets[PILOT_WEAPON_SETS]; /**< All the weapon sets the pilot has. */
   int active_set;   /**< Index of the currently active weapon set. */
   int autoweap;     /**< Automatically update weapon sets. */
   int aimLines;     /**< Activate aiming helper lines. */

   /* Cargo */
   credits_t credits; /**< monies the pilot has */
   PilotCommodity* commodities; /**< commodity and quantity */
   int cargo_free;   /**< Free commodity space. */

   /* Hook attached to the pilot */
   PilotHook *hooks; /**< Array (array.h): Pilot hooks. */

   /* Escort stuff. */
   unsigned int parent; /**< Pilot's parent. */
   Escort_t *escorts;   /**< Array (array.h): Pilot's escorts. */
   unsigned int dockpilot;/**< Pilot's dock pilot (the pilot it originates from). This is
                          separate from parent because it needs to be set in sync with
                          dockslot (below). Used to unset dockslot when the dock pilot
                          is destroyed. */
   int dockslot; /**< Outfit slot pilot originates from, index of dockpilot's outfits. */

   /* Targeting. */
   unsigned int target; /**< AI pilot target. */
   void *ptarget;       /**< AI pilot real target. */
   int nav_spob;      /**< Spob land target. */
   int nav_hyperspace;  /**< Hyperspace target. */
   int nav_anchor;      /**< Asteroid anchor target. */
   int nav_asteroid;    /**< Asteroid target. */

   /* AI */
   AI_Profile* ai;   /**< AI personality profile */
   int lua_mem;      /**< AI memory. */
   double tcontrol;  /**< timer for control tick */
   double timer[MAX_AI_TIMERS]; /**< Timers for AI */
   Task* task;       /**< current action */
   unsigned int shoot_indicator; /**< Indicator to inform the AI if a seeker has been shot recently. */

   /* Ship Lua. */
   int lua_ship_mem; /**< Ship memory. */

   /* Misc */
   double comm_msgTimer; /**< Message timer for the comm. */
   double comm_msgWidth; /**< Width of the message. */
   char *comm_msg;   /**< Comm message to display overhead. */
   PilotFlags flags; /**< used for AI and others */
   double landing_delay;/**< This pilot's current landing delay. */
   double pdata;     /**< generic data for internal pilot use */
   double ptimer;    /**< generic timer for internal pilot use */
   double itimer;    /**< Timer for player invulnerability. */
   double htimer;    /**< Hail animation timer. */
   double stimer;    /**< Shield regeneration timer. */
   double sbonus;    /**< Shield regeneration bonus. */
   double dtimer;    /**< Disable timer. */
   double dtimer_accum; /**< Accumulated disable timer. */
   double otimer;    /**< Lua outfit timer. */
   double scantimer; /**< Electronic warfare scanning timer. */
   int hail_pos;     /**< Hail animation position. */
   int lockons;      /**< Stores how many seeking weapons are targeting pilot */
   int projectiles;  /**< Stores how many weapons are after the pilot */
   int *mounted;     /**< Number of mounted outfits on the mount. */
   double player_damage;/**< Accumulates damage done by player for hostileness.
                             In per one of max shield + armour. */
   double engine_glow;/**< Amount of engine glow to display. */
   int messages;     /**< Queued messages (Lua ref). */
   lvar *shipvar;    /**< Per-ship version of lua mission variables. */
} Pilot;

/* These depend on Pilot being defined first. */
#include "pilot_cargo.h"
#include "pilot_heat.h"
#include "pilot_hook.h"
#include "pilot_outfit.h"
#include "pilot_weapon.h"
#include "pilot_ew.h"

/*
 * Getting pilot stuff.
 */
Pilot*const* pilot_getAll (void);
Pilot* pilot_get( unsigned int id );
Pilot* pilot_getTarget( Pilot *p );
unsigned int pilot_getNextID( unsigned int id, int mode );
unsigned int pilot_getPrevID( unsigned int id, int mode );
unsigned int pilot_getNearestEnemy( const Pilot* p );
unsigned int pilot_getNearestEnemy_size( const Pilot* p, double target_mass_LB, double target_mass_UB );
unsigned int pilot_getNearestEnemy_heuristic(const Pilot* p, double mass_factor, double health_factor, double damage_factor, double range_factor);
unsigned int pilot_getNearestHostile (void); /* only for the player */
unsigned int pilot_getNearestPilot( const Pilot* p );
unsigned int pilot_getBoss( const Pilot* p );
double pilot_getNearestPos( const Pilot *p, unsigned int *tp, double x, double y, int disabled );
double pilot_getNearestAng( const Pilot *p, unsigned int *tp, double ang, int disabled );
int pilot_getJumps( const Pilot* p );
const glColour* pilot_getColour( const Pilot* p );
int pilot_validTarget( const Pilot* p, const Pilot* target );
int pilot_canTarget( const Pilot* p );

/* non-lua wrappers */
double pilot_relsize( const Pilot* cur_pilot, const Pilot* p );
double pilot_reldps( const Pilot* cur_pilot, const Pilot* p );
double pilot_relhp( const Pilot* cur_pilot, const Pilot* p );

/*
 * Combat.
 */
void pilot_setTarget( Pilot* p, unsigned int id );
double pilot_hit( Pilot* p, const Solid* w, const Pilot *pshooter,
      const Damage *dmg, const Outfit *outfit, int lua_mem, int reset );
void pilot_updateDisable( Pilot* p, unsigned int shooter );
void pilot_explode( double x, double y, double radius, const Damage *dmg, const Pilot *parent );
double pilot_face( Pilot* p, const double dir );
int pilot_brake( Pilot* p );
double pilot_brakeDist( Pilot *p, vec2 *pos );
int pilot_interceptPos( Pilot *p, double x, double y );
void pilot_cooldown( Pilot *p, int dochecks );
void pilot_cooldownEnd( Pilot *p, const char *reason );
double pilot_aimAngle( Pilot *p, const vec2* pos, const vec2* vel );

/*
 * Faction stuff.
 */
int pilot_validEnemy( const Pilot* p, const Pilot* target );
int pilot_validEnemyDist( const Pilot* p, const Pilot* target, double *dist );
int pilot_areAllies( const Pilot *p, const Pilot *target );
int pilot_areEnemies( const Pilot *p, const Pilot *target );

/* Outfits */
int pilot_numOutfit( const Pilot *p, const Outfit *o );
void pilot_dpseps( const Pilot *p, double *pdps, double *peps );

/* Misc. */
int pilot_hasCredits( Pilot *p, credits_t amount );
credits_t pilot_modCredits( Pilot *p, credits_t amount );
int pilot_refuelStart( Pilot *p );
void pilot_hyperspaceAbort( Pilot* p );
void pilot_clearTimers( Pilot *pilot );
int pilot_hasDeployed( Pilot *p );
int pilot_dock( Pilot *p, Pilot *target );
ntime_t pilot_hyperspaceDelay( Pilot *p );
void pilot_untargetAsteroid( int anchor, int asteroid );
PilotOutfitSlot* pilot_getDockSlot( Pilot* p );

/*
 * Creation.
 */
unsigned int pilot_create( const Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const vec2* pos, const vec2* vel,
      const PilotFlags flags, unsigned int dockpilot, int dockslot );
Pilot* pilot_createEmpty( const Ship* ship, const char* name,
      int faction, PilotFlags flags );
unsigned int pilot_clone( const Pilot *p );
unsigned int pilot_addStack( Pilot *p );
void pilot_reset( Pilot* pilot );
Pilot* pilot_setPlayer( Pilot* after );
void pilot_choosePoint( vec2 *vp, Spob **spob, JumpPoint **jump, int lf, int ignore_rules, int guerilla );
void pilot_delete( Pilot *p );
void pilot_dead( Pilot* p, unsigned int killer );

/*
 * Init and cleanup.
 */
void pilot_stackRemove( Pilot *p );
void pilots_init (void);
void pilots_free (void);
void pilots_clean( int persist );
void pilots_newSystem (void);
void pilots_clear (void);
void pilots_cleanAll (void);
void pilot_free( Pilot* p );

/*
 * Movement.
 */
void pilot_setThrust( Pilot *p, double thrust );
void pilot_setTurn( Pilot *p, double turn );

/*
 * update
 */
void pilot_update( Pilot* pilot, double dt );
void pilots_update( double dt );
void pilot_renderFramebuffer( Pilot *p, GLuint fbo, double fw, double fh );
void pilots_render (void);
void pilots_renderOverlay (void);
void pilot_render( Pilot* pilot );
void pilot_renderOverlay( Pilot* p );

/*
 * communication
 */
void pilot_broadcast( Pilot *p, const char *msg, int ignore_int );
void pilot_distress( Pilot *p, Pilot *attacker, const char *msg );
void pilot_setCommMsg( Pilot *p, const char *s );

/*
 * faction
 */
void pilot_setHostile( Pilot *p );
void pilot_rmHostile( Pilot *p );
void pilot_setFriendly( Pilot *p );
void pilot_rmFriendly( Pilot *p );
int pilot_isHostile( const Pilot *p );
int pilot_isNeutral( const Pilot *p );
int pilot_isFriendly( const Pilot *p );
char pilot_getFactionColourChar( const Pilot *p );

/*
 * Misc details.
 */
credits_t pilot_worth( const Pilot *p );
void pilot_msg( Pilot *p, Pilot *receiver, const char *type, unsigned int index );
void pilot_clearTrails( Pilot *p );
void pilot_sample_trails( Pilot* p, int none );
int pilot_hasIllegal( const Pilot *p, int faction );
