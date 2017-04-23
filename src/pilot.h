/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PILOT_H
#  define PILOT_H


#include "physics.h"
#include "ship.h"
#include "ai.h"
#include "outfit.h"
#include "faction.h"
#include "sound.h"
#include "economy.h"
#include "ntime.h"


#define PLAYER_ID       1 /**< Player pilot ID. */


/* Hyperspace parameters. */
#define HYPERSPACE_ENGINE_DELAY  3. /**< Time to warm up engine (seconds). */
#define HYPERSPACE_FLY_DELAY     5. /**< Time it takes to hyperspace (seconds). */
#define HYPERSPACE_STARS_BLUR    3. /**< How long it takes for stars to start blurring (seconds). */
#define HYPERSPACE_STARS_LENGTH  250 /**< Length the stars blur to at max (pixels). */
#define HYPERSPACE_FADEOUT       1. /**< How long the fade is (seconds). */
#define HYPERSPACE_THRUST        2000./**< How much thrust you use in hyperspace. */
#define HYPERSPACE_VEL           2 * HYPERSPACE_THRUST*HYPERSPACE_FLY_DELAY /**< Velocity at hyperspace. */
#define HYPERSPACE_ENTER_MIN     HYPERSPACE_VEL*0.3 /**< Minimum entering distance. */
#define HYPERSPACE_ENTER_MAX     HYPERSPACE_VEL*0.4 /**< Maximum entering distance. */
#define HYPERSPACE_EXIT_MIN      1500. /**< Minimum distance to begin jumping. */
/* Land/takeoff. */
#define PILOT_LANDING_DELAY      2. /**< Delay for land animation. */
#define PILOT_TAKEOFF_DELAY      2. /**< Delay for takeoff animation. */
/* Refueling. */
#define PILOT_REFUEL_TIME        3. /**< Time to complete refueling. */
#define PILOT_REFUEL_QUANTITY    100. /**< Amount transferred per refuel. */
#define PILOT_REFUEL_RATE        PILOT_REFUEL_QUANTITY/PILOT_REFUEL_TIME /**< Fuel per second. */
/* Misc. */
#define PILOT_SIZE_APROX         0.8   /**< approximation for pilot size */
#define PILOT_WEAPON_SETS        10    /**< Number of weapon sets the pilot has. */
#define PILOT_WEAPSET_MAX_LEVELS 2     /**< Maximum amount of weapon levels. */
#define PILOT_REVERSE_THRUST     0.4   /**< Ratio of normal thrust to apply when reversing. */


/* hooks */
enum {
   PILOT_HOOK_NONE,      /**< No hook. */
   PILOT_HOOK_DEATH,     /**< Pilot died. */
   PILOT_HOOK_BOARDING,  /**< Pilot is boarding. */
   PILOT_HOOK_BOARD,     /**< Pilot got boarded. */
   PILOT_HOOK_DISABLE,   /**< Pilot got disabled. */
   PILOT_HOOK_UNDISABLE, /**< Pilot recovered from being disabled. */
   PILOT_HOOK_JUMP,      /**< Pilot jumped. */
   PILOT_HOOK_HAIL,      /**< Pilot is hailed. */
   PILOT_HOOK_LAND,      /**< Pilot is landing. */
   PILOT_HOOK_ATTACKED,  /**< Pilot is in manual override and is being attacked. */
   PILOT_HOOK_IDLE,      /**< Pilot is in manual override and has just become idle. */
   PILOT_HOOK_EXPLODED,  /**< Pilot died and exploded (about to be removed). */
   PILOT_HOOK_LOCKON     /**< Pilot had a launcher lockon. */
};


/* damage */
#define PILOT_HOSTILE_THRESHOLD  0.09 /**< Point at which pilot becomes hostile. */
#define PILOT_HOSTILE_DECAY      0.005 /**< Rate at which hostility decays. */


/* flags */
#define pilot_clearFlagsRaw(a) memset((a), 0, PILOT_FLAGS_MAX) /**< Clears the pilot flags. */
#define pilot_copyFlagsRaw(d,s) memcpy((d), (s), PILOT_FLAGS_MAX) /**< Copies the pilot flags from s to d. */
#define pilot_isFlagRaw(a,f)  ((a)[f]) /**< Checks to see if a pilot flag is set. */
#define pilot_setFlagRaw(a,f) ((a)[f] = 1) /**< Sets flags rawly. */
#define pilot_isFlag(p,f)     ((p)->flags[f]) /**< Checks if flag f is set on pilot p. */
#define pilot_setFlag(p,f)    ((p)->flags[f] = 1) /**< Sets flag f on pilot p. */
#define pilot_rmFlag(p,f)     ((p)->flags[f] = 0) /**< Removes flag f on pilot p. */
enum {
   /* creation */
   PILOT_PLAYER,       /**< Pilot is a player. */
   PILOT_CARRIED,      /**< Pilot usually resides in a fighter bay. */
   PILOT_CREATED_AI,   /** Pilot has already created AI. */
   PILOT_EMPTY,        /**< Do not add pilot to stack. */
   PILOT_NO_OUTFITS,   /**< Do not create the pilot with outfits. */
   /* dynamic */
   PILOT_HAILING,      /**< Pilot is hailing the player. */
   PILOT_NODISABLE,    /**< Pilot can't be disabled. */
   PILOT_INVINCIBLE,   /**< Pilot can't be hit ever. */
   PILOT_HOSTILE,      /**< Pilot is hostile to the player. */
   PILOT_FRIENDLY,     /**< Pilot is friendly to the player. */
   PILOT_COMBAT,       /**< Pilot is engaged in combat. */
   PILOT_AFTERBURNER,  /**< Pilot has their afterburner activated. */
   PILOT_HYP_PREP,     /**< Pilot is getting ready for hyperspace. */
   PILOT_HYP_BRAKE,    /**< PIlot has already braked before jumping. */
   PILOT_HYP_BEGIN,    /**< Pilot is starting engines. */
   PILOT_HYPERSPACE,   /**< Pilot is in hyperspace. */
   PILOT_HYP_END,      /**< Pilot is exiting hyperspace. */
   PILOT_BOARDED,      /**< Pilot has been boarded already. */
   PILOT_NOBOARD,      /**< Pilot can't be boarded. */
   PILOT_BOARDING,     /**< Pilot is currently boarding it's target. */
   PILOT_BRIBED,       /**< Pilot has been bribed already. */
   PILOT_DISTRESSED,   /**< Pilot has distressed once already. */
   PILOT_REFUELING,    /**< Pilot is trying to refueling. */
   PILOT_REFUELBOARDING, /**< Pilot is actively refueling. */
   PILOT_MANUAL_CONTROL, /**< Pilot is under manual control of a mission or event. */
   PILOT_LANDING,      /**< Pilot is landing. */
   PILOT_TAKEOFF,      /**< Pilot is taking off. */
   PILOT_DISABLED,     /**< Pilot is disabled. */
   PILOT_DISABLED_PERM, /**< Pilot is permanently disabled. */
   PILOT_DEAD,         /**< Pilot is in it's dying throes */
   PILOT_DEATH_SOUND,  /**< Pilot just did death explosion. */
   PILOT_EXPLODED,     /**< Pilot did final death explosion. */
   PILOT_DELETE,       /**< Pilot will get deleted asap. */
   PILOT_VISPLAYER,    /**< Pilot is always visible to the player (only player). */
   PILOT_VISIBLE,      /**< Pilot is always visible to other pilots. */
   PILOT_HILIGHT,      /**< Pilot is hilighted when visible (this does not increase visibility). */
   PILOT_INVISIBLE,    /**< Pilot is invisible to other pilots. */
   PILOT_BOARDABLE,    /**< Pilot can be boarded even while active. */
   PILOT_NOJUMP,       /**< Pilot cannot engage hyperspace engines. */
   PILOT_NOLAND,       /**< Pilot cannot land on stations or planets. */
   PILOT_NODEATH,      /**< Pilot can not die, will stay at 1 armour. */
   PILOT_INVINC_PLAYER, /**< Pilot can not be hurt by the player. */
   PILOT_COOLDOWN,     /**< Pilot is in active cooldown mode. */
   PILOT_COOLDOWN_BRAKE, /**< Pilot is braking to enter active cooldown mode. */
   PILOT_BRAKING,      /**< Pilot is braking. */
   PILOT_HASSPEEDLIMIT, /**< Speed limiting is activated for Pilot.*/
   PILOT_FLAGS_MAX     /**< Maximum number of flags. */
};
typedef char PilotFlags[ PILOT_FLAGS_MAX ];

/* makes life easier */
#define pilot_isPlayer(p)   pilot_isFlag(p,PILOT_PLAYER) /**< Checks if pilot is a player. */
#define pilot_isDisabled(p) pilot_isFlag(p,PILOT_DISABLED) /**< Checks if pilot is disabled. */
#define pilot_isStopped(p)  (VMOD(p->solid->vel) <= MIN_VEL_ERR)


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
   Outfit *outfit;      /**< Type of ammo. */
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
   Outfit* outfit;   /**< Associated outfit. */
   int active;       /**< Slot is an active slot. */
   ShipOutfitSlot *sslot; /**< Ship outfit slot. */

   /* Heat. */
   double heat_T;    /**< Slot temperature. [K] */
   double heat_C;    /**< Slot heat capacity. [W/K] */
   double heat_area; /**< Slot area of contact with ship hull. [m^2] */
   double heat_start; /**< Slot heat at the beginning of a cooldown period. */

   /* Current state. */
   PilotOutfitState state; /**< State of the outfit. */
   double stimer;    /**< State timer, tracking current state. */
   double timer;     /**< Used to store when it was last used. */
   int level;        /**< Level in current weapon set (-1 is none). */
   int weapset;      /**< First weapon set that uses the outfit (-1 is none). */

   /* Type-specific data. */
   union {
      unsigned int beamid;    /**< ID of the beam used in this outfit, only used for beams. */
      PilotOutfitAmmo ammo;   /**< Ammo for launchers. */
   } u; /**< Stores type specific data. */
} PilotOutfitSlot;


/**
 * @brief A pilot Weapon Set Outfit.
 */
typedef struct PilotWeaponSetOutfit_ {
   int level;              /**< Level of trigger. */
   double range2;          /**< Range squared of this specific outfit. */
   PilotOutfitSlot *slot;  /**< Slot associated with it. */
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
   double range[PILOT_WEAPSET_MAX_LEVELS]; /**< Range of the levels in the outfit slot. */
   double speed[PILOT_WEAPSET_MAX_LEVELS]; /**< Speed of the levels in the outfit slot. */
} PilotWeaponSet;


/**
 * @brief Stores a pilot commodity.
 */
typedef struct PilotCommodity_ {
   Commodity* commodity;   /**< Associated commodity. */
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
   ESCORT_TYPE_ALLY        /**< Escort is an ally, uncontrollable. */
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
   char* title;      /**< title - usually indicating special properties - @todo use */

   /* Fleet/faction management. */
   int faction;      /**< Pilot's faction. */
   int presence;     /**< Presence being used by the pilot. */

   /* Object characteristics */
   Ship* ship;       /**< ship pilot is flying */
   Solid* solid;     /**< associated solid (physics) */
   double base_mass; /**< Ship mass plus core outfit mass. */
   double mass_cargo; /**< Amount of cargo mass added. */
   double mass_outfit; /**< Amount of outfit mass added. */
   int tsx;          /**< current sprite x position, calculated on update. */
   int tsy;          /**< current sprite y position, calculated on update. */

   /* Properties. */
   int cpu;       /**< Amount of CPU the pilot has left. */
   int cpu_max;   /**< Maximum amount of CPU the pilot has. */
   double crew;      /**< Crew amount the player has (display it as (int)floor(), but it's analogue. */
   double cap_cargo; /**< Pilot's cargo capacity. */

   /* Movement */
   double thrust;    /**< Pilot's thrust in px/s^2. */
   double thrust_base; /**< Pilot's base thrust in px/s^2 (not modulated by mass). */
   double speed;     /**< Pilot's speed in px/s. */
   double speed_base; /**< Pilot's base speed in px/s (not modulated by mass). */
   double speed_limit; /**< Pilot's maximum speed in px/s if limited by lua call. */
   double turn;      /**< Pilot's turn in rad/s. */
   double turn_base; /**< Pilot's base turn in rad/s (not modulated by mass). */

   /* Current health */
   double armour;    /**< Current armour. */
   double stress;    /**< Current disable damage level. */
   double shield;    /**< Current shield. */
   double fuel;      /**< Current fuel. */
   double fuel_consumption; /**< Fuel consumed per jump. */
   double armour_max; /**< Maximum armour. */
   double shield_max; /**< Maximum shield. */
   double fuel_max;  /**< Maximum fuel. */
   double armour_regen; /**< Armour regeneration rate (per second). */
   double shield_regen; /**< Shield regeneration rate (per second). */
   double dmg_absorb; /**< Ship damage absorption [0:1] with 1 being 100%. */

   /* Energy is handled a bit differently. */
   double energy;    /**< Current energy. */
   double energy_max; /**< Maximum energy. */
   double energy_regen; /**< Energy regeneration rate (per second). */
   double energy_tau; /**< Tau regeneration rate for energy. */
   double energy_loss; /**< Linear loss that bypasses the actual RC circuit stuff. */

   /* Electronic warfare. */
   double ew_base_hide; /**< Base static hide factor. */
   double ew_mass;   /**< Mass factor. */
   double ew_heat;   /**< Heat factor, affects hide. */
   double ew_asteroid;   /**< Asteroid field factor, affects hide. */
   double ew_hide;   /**< Static hide factor. */
   double ew_movement; /**< Movement factor. */
   double ew_evasion; /**< Dynamic evasion factor. */
   double ew_detect; /**< Static detection factor. */
   double ew_jump_detect; /** Static jump detection factor */

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
   ShipStats stats;  /**< Pilot's copy of ship statistics. */

   /* Associated functions */
   void (*think)(struct Pilot_*, const double); /**< AI thinking for the pilot */
   void (*update)(struct Pilot_*, const double); /**< Updates the pilot. */
   void (*render)(struct Pilot_*, const double); /**< For rendering the pilot. */
   void (*render_overlay)(struct Pilot_*, const double); /**< For rendering the pilot overlay. */

   /* Outfit management */
   /* Global outfits. */
   int noutfits;     /**< Total amount of slots. */
   PilotOutfitSlot **outfits; /**< Total outfits. */
   /* Per slot types. */
   int outfit_nstructure; /**< Number of structure slots. */
   PilotOutfitSlot *outfit_structure; /**< The structure slots. */
   int outfit_nutility; /**< Number of utility slots. */
   PilotOutfitSlot *outfit_utility; /**< The utility slots. */
   int outfit_nweapon; /**< Number of weapon slots. */
   PilotOutfitSlot *outfit_weapon; /**< The weapon slots. */

   /* Primarily for AI usage. */
   int ncannons;      /**< Number of cannons equipped. */
   int nturrets;      /**< Number of turrets equipped. */
   int nbeams;        /**< Number of beams equipped. */
   int njammers;      /**< Number of jammers equipped. */
   int nafterburners; /**< Number of afterburners equipped. */

   /* For easier usage. */
   PilotOutfitSlot *afterburner; /**< the afterburner */

   /* Jamming */
   int jamming;      /**< Pilot is current jamming with at least a single jammer (used to
                          speed up later checks in the code). */

   /* Weapon sets. */
   PilotWeaponSet weapon_sets[PILOT_WEAPON_SETS]; /**< All the weapon sets the pilot has. */
   int active_set;   /**< Index of the currently active weapon set. */
   int autoweap;     /**< Automatically update weapon sets. */

   /* Cargo */
   credits_t credits; /**< monies the pilot has */
   PilotCommodity* commodities; /**< commodity and quantity */
   int ncommodities; /**< number of commodities. */
   int cargo_free;   /**< Free commodity space. */

   /* Hook attached to the pilot */
   PilotHook *hooks; /**< Pilot hooks. */
   int nhooks;       /**< Number of pilot hooks. */

   /* Escort stuff. */
   unsigned int parent; /**< Pilot's parent. */
   Escort_t *escorts; /**< Pilot's escorts. */
   int nescorts;     /**< Number of pilot escorts. */

   /* Targeting. */
   unsigned int target; /**< AI pilot target. */
   int nav_planet;   /**< Planet land target. */
   int nav_hyperspace; /**< Hyperspace target. */

   /* AI */
   AI_Profile* ai;   /**< AI personality profile */
   double tcontrol;  /**< timer for control tick */
   double timer[MAX_AI_TIMERS]; /**< timers for AI */
   Task* task;       /**< current action */

   /* Misc */
   double comm_msgTimer; /**< Message timer for the comm. */
   double comm_msgWidth; /**< Width of the message. */
   char *comm_msg;   /**< Comm message to display overhead. */
   PilotFlags flags; /**< used for AI and others */
   double pdata;     /**< generic data for internal pilot use */
   double ptimer;    /**< generic timer for internal pilot use */
   double htimer;    /**< Hail animation timer. */
   double stimer;    /**< Shield regeneration timer. */
   double sbonus;    /**< Shield regeneration bonus. */
   double dtimer;    /**< Disable timer. */
   double dtimer_accum; /**< Accumulated disable timer. */
   int hail_pos;     /**< Hail animation position. */
   int lockons;      /**< Stores how many seeking weapons are targeting pilot */
   int *mounted;     /**< Number of mounted outfits on the mount. */
   double player_damage; /**< Accumulates damage done by player for hostileness.
                              In per one of max shield + armour. */
   double engine_glow; /**< Amount of engine glow to display. */
   int messages;       /**< Queued messages (Lua ref). */
} Pilot;


#include "pilot_cargo.h"
#include "pilot_heat.h"
#include "pilot_hook.h"
#include "pilot_outfit.h"
#include "pilot_weapon.h"
#include "pilot_ew.h"


/*
 * getting pilot stuff
 */
Pilot** pilot_getAll( int *n );
Pilot* pilot_get( const unsigned int id );
unsigned int pilot_getNextID( const unsigned int id, int mode );
unsigned int pilot_getPrevID( const unsigned int id, int mode );
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

/* non-lua wrappers */
double pilot_relsize( const Pilot* cur_pilot, const Pilot* p );
double pilot_reldps( const Pilot* cur_pilot, const Pilot* p );
double pilot_relhp( const Pilot* cur_pilot, const Pilot* p );

/*
 * Combat.
 */
void pilot_setTarget( Pilot* p, unsigned int id );
double pilot_hit( Pilot* p, const Solid* w, const unsigned int shooter,
      const Damage *dmg, int reset );
void pilot_updateDisable( Pilot* p, const unsigned int shooter );
void pilot_explode( double x, double y, double radius, const Damage *dmg, const Pilot *parent );
double pilot_face( Pilot* p, const double dir );
int pilot_brake( Pilot* p );
double pilot_brakeDist( Pilot *p, Vector2d *pos );
int pilot_interceptPos( Pilot *p, double x, double y );
void pilot_cooldown( Pilot *p );
void pilot_cooldownEnd( Pilot *p, const char *reason );


/* Misc. */
int pilot_hasCredits( Pilot *p, credits_t amount );
credits_t pilot_modCredits( Pilot *p, credits_t amount );
int pilot_refuelStart( Pilot *p );
void pilot_hyperspaceAbort( Pilot* p );
void pilot_clearTimers( Pilot *pilot );
int pilot_hasDeployed( Pilot *p );
int pilot_dock( Pilot *p, Pilot *target, int deployed );
ntime_t pilot_hyperspaceDelay( Pilot *p );


/*
 * creation
 */
void pilot_init( Pilot* dest, Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const PilotFlags flags );
unsigned int pilot_create( Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const PilotFlags flags );
Pilot* pilot_createEmpty( Ship* ship, const char* name,
      int faction, const char *ai, PilotFlags flags );
Pilot* pilot_copy( Pilot* src );
void pilot_delete( Pilot *p );


/*
 * init/cleanup
 */
void pilot_destroy(Pilot* p);
void pilots_free (void);
void pilots_clean (void);
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
void pilot_update( Pilot* pilot, const double dt );
void pilots_update( double dt );
void pilots_render( double dt );
void pilots_renderOverlay( double dt );
void pilot_render( Pilot* pilot, const double dt );
void pilot_renderOverlay( Pilot* p, const double dt );


/*
 * communication
 */
void pilot_message( Pilot *p, unsigned int target, const char *msg, int ignore_int );
void pilot_broadcast( Pilot *p, const char *msg, int ignore_int );
void pilot_distress( Pilot *p, Pilot *attacker, const char *msg, int ignore_int );


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
void pilot_msg(Pilot *p, Pilot *reciever, const char *type, unsigned int index);

#endif /* PILOT_H */
