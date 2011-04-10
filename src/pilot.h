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
#define HYPERSPACE_STARS_BLUR    3. /**< How long the stars blur at max (pixels). */
#define HYPERSPACE_STARS_LENGTH  250 /**< Length the stars blur to at max (pixels). */
#define HYPERSPACE_FADEOUT       1. /**< How long the fade is (seconds). */
#define HYPERSPACE_FUEL          100.  /**< how much fuel it takes */
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
#define PILOT_REFUEL_RATE        HYPERSPACE_FUEL/PILOT_REFUEL_TIME /**< Fuel per second. */
/* Misc. */
#define PILOT_SIZE_APROX         0.8   /**< approximation for pilot size */
#define PILOT_DISABLED_ARMOR     0.3   /**< armour % that gets it disabled */
#define PILOT_WEAPON_SETS        10    /**< Number of weapon sets the pilot has. */
#define PILOT_WEAPSET_MAX_LEVELS 2     /**< Maxmimum amount of weapon levels. */

/* hooks */
#define PILOT_HOOK_NONE    0 /**< No hook. */
#define PILOT_HOOK_DEATH   1 /**< Pilot died. */
#define PILOT_HOOK_BOARD   2 /**< Pilot got boarded. */
#define PILOT_HOOK_DISABLE 3 /**< Pilot got disabled. */
#define PILOT_HOOK_JUMP    4 /**< Pilot jumped. */
#define PILOT_HOOK_HAIL    5 /**< Pilot is hailed. */
#define PILOT_HOOK_LAND    6 /**< Pilot is landing. */
#define PILOT_HOOK_ATTACKED 7 /**< Pilot is in manual override and is being attacked. */
#define PILOT_HOOK_IDLE    8 /**< Pilot is in manual override and has just become idle. */
#define PILOT_HOOK_EXPLODED 9 /**< Pilot died and exploded (about to be removed). */


/* damage */
#define PILOT_HOSTILE_THRESHOLD  0.09 /**< Point at which pilot becomes hostile. */
#define PILOT_HOSTILE_DECAY      0.005 /**< Rate at which hostility decays. */


/* flags */
#define pilot_clearFlagsRaw(a) memset((a), 0, PILOT_FLAGS_MAX) /**< Clears the pilot flags. */
#define pilot_isFlagRaw(a,f)  ((a)[f]) /**< Checks to see if a pilot flag is set. */
#define pilot_setFlagRaw(a,f) ((a)[f] = 1) /**< Sets flags rawly. */
#define pilot_isFlag(p,f)     ((p)->flags[f]) /**< Checks if flag f is set on pilot p. */
#define pilot_setFlag(p,f)    ((p)->flags[f] = 1) /**< Sets flag f on pilot p. */
#define pilot_rmFlag(p,f)     ((p)->flags[f] = 0) /**< Removes flag f on pilot p. */
/* creation */
#define PILOT_PLAYER       0 /**< Pilot is a player. */
#define PILOT_ESCORT       1 /**< Pilot is an escort. */
#define PILOT_CARRIED      2 /**< Pilot usually resides in a fighter bay. */
#define PILOT_CREATED_AI   3 /** Pilot has already created AI. */
#define PILOT_EMPTY        4 /**< Do not add pilot to stack. */
#define PILOT_NO_OUTFITS   5 /**< Do not create the pilot with outfits. */
#define PILOT_HASTURRET    6 /**< Pilot has turrets. */
#define PILOT_HASBEAMS     7 /**< Pilot has beam weapons. */
/* dynamic */
#define PILOT_HAILING      8 /**< Pilot is hailing the player. */
#define PILOT_NODISABLE    9 /**< Pilot can't be disabled. */
#define PILOT_INVINCIBLE   10 /**< Pilot can't be hit ever. */
#define PILOT_HOSTILE      11 /**< Pilot is hostile to the player. */
#define PILOT_FRIENDLY     12 /**< Pilot is friendly to the player. */
#define PILOT_COMBAT       13 /**< Pilot is engaged in combat. */
#define PILOT_AFTERBURNER  14 /**< Pilot has his afterburner activated. */
#define PILOT_HYP_PREP     15 /**< Pilot is getting ready for hyperspace. */
#define PILOT_HYP_BRAKE    16 /**< PIlot has already braked before jumping. */
#define PILOT_HYP_BEGIN    17 /**< Pilot is starting engines. */
#define PILOT_HYPERSPACE   18 /**< Pilot is in hyperspace. */
#define PILOT_HYP_END      19 /**< Pilot is exiting hyperspace. */
#define PILOT_BOARDED      20 /**< Pilot has been boarded already. */
#define PILOT_NOBOARD      21 /**< Pilot can't be boarded. */
#define PILOT_BOARDING     22 /**< Pilot is currently boarding it's target. */
#define PILOT_BRIBED       23 /**< Pilot has been bribed already. */
#define PILOT_DISTRESSED   24 /**< Pilot has distressed once already. */
#define PILOT_REFUELING    25 /**< Pilot is trying to refueling. */
#define PILOT_REFUELBOARDING 26 /**< Pilot is actively refueling. */
#define PILOT_MANUAL_CONTROL 27 /**< Pilot is under manual control of a mission or event. */
#define PILOT_LANDING      28 /**< Pilot is landing. */
#define PILOT_TAKEOFF      29 /**< Pilot is taking off. */
#define PILOT_DISABLED     30 /**< Pilot is disabled. */
#define PILOT_DEAD         31 /**< Pilot is in it's dying throes */
#define PILOT_DEATH_SOUND  32 /**< Pilot just did death explosion. */
#define PILOT_EXPLODED     33 /**< Pilot did final death explosion. */
#define PILOT_DELETE       34 /**< Pilot will get deleted asap. */
#define PILOT_VISPLAYER    35 /**< Pilot is always visible to the player (only player). */
#define PILOT_VISIBLE      36 /**< Pilot is always visible to other pilots. */
#define PILOT_HILIGHT      37 /**< Pilot is hilighted when visible (this does not increase visibility). */
#define PILOT_INVISIBLE    38 /**< Pilot is invisible to other pilots. */
#define PILOT_BOARDABLE    39 /**< Pilot can be boarded even while active. */
#define PILOT_NOJUMP       40 /**< Pilot cannot engage hyperspace engines. */
#define PILOT_NOLAND       41 /**< Pilot cannot land on stations or planets. */
#define PILOT_FLAGS_MAX    PILOT_NOLAND+1 /* Maximum number of flags. */
typedef char PilotFlags[ PILOT_FLAGS_MAX ];

/* makes life easier */
#define pilot_isPlayer(p)   pilot_isFlag(p,PILOT_PLAYER) /**< Checks if pilot is a player. */
#define pilot_isDisabled(p) pilot_isFlag(p,PILOT_DISABLED) /**< Checks if pilot is disabled. */


/**
 * @brief Contains the state of the outfit.
 *
 * Currently only applicable to beam weapons.
 */
typedef enum PilotOutfitState_ {
   PILOT_OUTFIT_OFF, /**< Normal state. */
   PILOT_OUTFIT_WARMUP, /**< Outfit is starting to warm up. */
   PILOT_OUTFIT_ON /**< Outfit is activated and running. */
} PilotOutfitState;


/**
 * @brief Stores outfit ammo.
 */
typedef struct PilotOutfitAmmo_ {
   Outfit *outfit; /**< Type of ammo. */
   int quantity; /**< Amount of ammo. */
   int deployed; /**< For fighter bays. */
} PilotOutfitAmmo;


/**
 * @brief Stores an outfit the pilot has.
 */
typedef struct PilotOutfitSlot_ {
   int id; /**< Position in the global slot list. */

   /* Outfit slot properties. */
   Outfit* outfit; /**< Associated outfit. */
   ShipMount mount; /**< Outfit mountpoint. */
   OutfitSlot slot; /**< Outfit slot. */
   int active; /**< Slot is an active slot. */
   double heat_T; /**< Slot temperature. [K] */
   double heat_C; /**< Slot heat capacity. [W/K] */
   double heat_area; /**< Slot area of contact with ship hull. [m^2] */

   /* Current state. */
   PilotOutfitState state; /**< State of the outfit. */
   double timer; /**< Used to store when it was last used. */
   int quantity; /**< Quantity. */
   int level; /**< Level in current weapon set (-1 is none). */

   /* Type-specific data. */
   union {
      unsigned int beamid; /**< ID of the beam used in this outfit, only used for beams. */
      PilotOutfitAmmo ammo; /**< Ammo for launchers. */
   } u; /**< Stores type specific data. */
} PilotOutfitSlot;


/**
 * @brief A pilot Weapon Set Outfit.
 */
typedef struct PilotWeaponSetOutfit_ {
   int level; /**< Level of trigger. */
   double range2; /**< Range squared of this specific outfit. */
   PilotOutfitSlot *slot; /**< Slot assosciated with it. */
} PilotWeaponSetOutfit;


/**
 * @brief A weapon set represents a set of weapons that have an action.
 *
 * By default a weapon set indicates what weapons are enabled at a given time.
 *  However they can also be used to launch weapons.
 */
typedef struct PilotWeaponSet_ {
   char *name; /**< Helpful for the player. */
   int fire; /**< Whether to fire the weapons or just enable them. */
   int active; /**< Whether or not it's currently firing. */
   int inrange; /**< Whether or not to fire only if the target is inrange. */
   double range[PILOT_WEAPSET_MAX_LEVELS]; /**< Range of the levels in the outfit slot. */
   double speed[PILOT_WEAPSET_MAX_LEVELS]; /**< Speed of the levels in the outfit slot. */
   PilotWeaponSetOutfit *slots; /**< Slots involved with the weapon set. */
} PilotWeaponSet;


/**
 * @brief Stores a pilot commodity.
 */
typedef struct PilotCommodity_ {
   Commodity* commodity; /**< Associated commodity. */
   int quantity; /**< Amount player has. */
   unsigned int id; /**< Special mission id for cargo, 0 means none. */
} PilotCommodity;


/**
 * @brief A wrapper for pilot hooks.
 */
typedef struct PilotHook_ {
   int type; /**< Type of hook. */
   unsigned int id; /**< Hook ID associated with pilot hook. */
} PilotHook;


/**
 * @brief Different types of escorts.
 */
typedef enum EscortType_e {
   ESCORT_TYPE_NULL, /**< Invalid escort type. */
   ESCORT_TYPE_BAY, /**< Escort is from a fighter bay. */
   ESCORT_TYPE_MERCENARY, /**< Escort is a mercenary. */
   ESCORT_TYPE_ALLY /**< Escort is an ally. */
} EscortType_t;


/**
 * @brief Stores an escort.
 */
typedef struct Escort_s {
   char *ship; /**< Type of the ship escort is flying. */
   EscortType_t type; /**< Type of escort. */
   unsigned int id; /**< ID of in-game pilot. */
} Escort_t;


/**
 * @brief The representation of an in-game pilot.
 */
typedef struct Pilot_ {

   unsigned int id; /**< pilot's id, used for many functions */
   char* name; /**< pilot's name (if unique) */
   char* title; /**< title - usually indicating special properties - @todo use */

   /* Fleet/faction management. */
   int faction; /**< Pilot's faction. */
   int systemFleet; /**< The system fleet the pilot belongs to. */
   int presence; /**< Presence being used by the pilot. */

   /* Object characteristics */
   Ship* ship; /**< ship pilot is flying */
   Solid* solid; /**< associated solid (physics) */
   double mass_cargo; /**< Amount of cargo mass added. */
   double mass_outfit; /**< Amount of outfit mass added. */
   int tsx; /**< current sprite x position, calculated on update. */
   int tsy; /**< current sprite y position, calculated on update. */

   /* Properties. */
   double cpu; /**< Amount of CPU the pilot has left. */
   double cpu_max; /**< Maximum amount of CPU the pilot has. */
   double crew; /**< Crew amount the player has (display it as (int)floor(), but it's analogue. */

   /* Movement */
   double thrust; /**< Pilot's thrust. */
   double speed; /**< Pilot's speed. */
   double turn; /**< Pilot's turn in rad/s. */
   double turn_base; /**< Pilot's base turn in rad/s. */

   /* Current health */
   double armour; /**< Current armour. */
   double shield; /**< Current shield. */
   double fuel; /**< Current fuel. */
   double armour_max; /**< Maximum armour. */
   double shield_max; /**< Maximum shield. */
   double fuel_max; /**< Maximum fuel. */
   double armour_regen; /**< Armour regeneration rate (per second). */
   double shield_regen; /**< Shield regeneration rate (per second). */
   double dmg_absorb; /**< Ship damage absorption [0:1] with 1 being 100%. */

   /* Energy is handled a bit differently. */
   double energy; /**< Current energy. */
   double energy_max; /**< Maximum energy. */
   double energy_regen; /**< Energy regeneration rate (per second). */
   double energy_tau; /**< Tau regeneration rate for energy. */

   /* Electronic warfare. */
   double ew_base_hide; /**< Base static hide factor. */
   double ew_mass; /**< Mass factor. */
   double ew_heat; /**< Heat factor, affects hide. */
   double ew_hide; /**< Static hide factor. */
   double ew_movement; /**< Movement factor. */
   double ew_evasion; /**< Dynamic evasion factor. */
   double ew_detect; /**< Static detection factor. */

   /* Heat. */
   double heat_T; /**< Ship temperature. [K] */
   double heat_C; /**< Heat capacity of the ship. [W/K] */
   double heat_emis; /**< Ship epsilon parameter (emissivity). [adimensional 0:1] */
   double heat_cond; /**< Ship conductivity parameter. [W/(m*K)] */
   double heat_area; /**< Effective heatsink area of the ship. [m^2] */

   /* Ship statistics. */
   ShipStats stats; /**< Pilot's copy of ship statistics. */

   /* Associated functions */
   void (*think)(struct Pilot_*, const double); /**< AI thinking for the pilot */
   void (*update)(struct Pilot_*, const double); /**< Updates the pilot. */
   void (*render)(struct Pilot_*, const double); /**< For rendering the pilot. */
   void (*render_overlay)(struct Pilot_*, const double); /**< For rendering the pilot overlay. */

   /* Outfit management */
   /* Global outfits. */
   int noutfits; /**< Total amount of slots. */
   PilotOutfitSlot **outfits; /**< Total outfits. */
   /* Per slot types. */
   int outfit_nstructure; /**< Number of structure slots. */
   PilotOutfitSlot *outfit_structure; /**< The structure slots. */
   int outfit_nutility; /**< Number of utility slots. */
   PilotOutfitSlot *outfit_utility; /**< The utility slots. */
   int outfit_nweapon; /**< Number of weapon slots. */
   PilotOutfitSlot *outfit_weapon; /**< The weapon slots. */
   /* For easier usage. */
   PilotOutfitSlot *afterburner; /**< the afterburner */

   /* Jamming */
   double jam_range; /**< Range at which pilot starts jamming. */
   double jam_chance; /**< Jam chance. */

   /* Weapon sets. */
   PilotWeaponSet weapon_sets[PILOT_WEAPON_SETS]; /**< All the weapon sets the pilot has. */
   int active_set; /**< Index of the currently active weapon set. */
   int autoweap; /**< Automatically update weapon sets. */

   /* Cargo */
   credits_t credits; /**< monies the pilot has */
   PilotCommodity* commodities; /**< commodity and quantity */
   int ncommodities; /**< number of commodities. */
   int cargo_free; /**< Free commodity space. */

   /* Hook attached to the pilot */
   PilotHook *hooks; /**< Pilot hooks. */
   int nhooks; /**< Number of pilot hooks. */

   /* Escort stuff. */
   unsigned int parent; /**< Pilot's parent. */
   Escort_t *escorts; /**< Pilot's escorts. */
   int nescorts; /**< Number of pilot escorts. */

   /* Targetting. */
   unsigned int target; /**< AI pilot target. */
   int nav_planet; /**< Planet land target. */
   int nav_hyperspace; /**< Hyperspace target. */

   /* AI */
   AI_Profile* ai; /**< ai personality profile */
   double tcontrol; /**< timer for control tick */
   double timer[MAX_AI_TIMERS]; /**< timers for AI */
   Task* task; /**< current action */

   /* Misc */
   double comm_msgTimer; /**< Message timer for the comm. */
   double comm_msgWidth; /**< Width of the message. */
   char *comm_msg; /**< Comm message to display overhead. */
   PilotFlags flags; /**< used for AI and others */
   double ptimer; /**< generic timer for internal pilot use */
   double htimer; /**< Hail animation timer. */
   double stimer; /**< Shield regeneration timer. */
   double sbonus; /**< Shield regeneration bonus. */
   int hail_pos; /**< Hail animation position. */
   int lockons; /**< Stores how many seeking weapons are targeting pilot */
   int *mounted; /**< Number of mounted outfits on the mount. */
   double player_damage; /**< Accumulates damage done by player for hostileness.
                              In per one of max shield + armour. */
   double engine_glow; /**< Amount of engine glow to display. */
} Pilot;


#include "pilot_cargo.h"
#include "pilot_heat.h"
#include "pilot_hook.h"
#include "pilot_outfit.h"
#include "pilot_weapon.h"
#include "pilot_cargo.h"
#include "pilot_ew.h"


/*
 * getting pilot stuff
 */
Pilot** pilot_getAll( int *n );
Pilot* pilot_get( const unsigned int id );
unsigned int pilot_getNextID( const unsigned int id, int mode );
unsigned int pilot_getPrevID( const unsigned int id, int mode );
unsigned int pilot_getNearestEnemy( const Pilot* p );
unsigned int pilot_getNearestEnemy_size( const Pilot* p, int target_mass_LB, int target_mass_UB );
unsigned int pilot_getNearestEnemy_heuristic(const Pilot* p, double mass_factor, double health_factor, double damage_factor, double range_factor);
unsigned int pilot_getNearestHostile (void); /* only for the player */
unsigned int pilot_getNearestPilot( const Pilot* p );
double pilot_getNearestPos( const Pilot *p, unsigned int *tp, double x, double y, int disabled );
double pilot_getNearestAng( const Pilot *p, unsigned int *tp, double ang, int disabled );
int pilot_getJumps( const Pilot* p );

/*non-lua wrappers*/
double pilot_relsize(const Pilot* cur_pilot, const Pilot* p);
double pilot_reldps(const Pilot* cur_pilot, const Pilot* p);
double pilot_relhp(const Pilot* cur_pilot, const Pilot* p);

/*
 * Combat.
 */
double pilot_hit( Pilot* p, const Solid* w, const unsigned int shooter,
      const DamageType dtype, const double damage, const double penetration );
void pilot_explode( double x, double y, double radius,
      DamageType dtype, double damage, 
      double penetration, const Pilot *parent );
double pilot_face( Pilot* p, const double dir );


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
      const PilotFlags flags, const int systemFleet );
unsigned int pilot_create( Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const PilotFlags flags, const int systemFleet );
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
void pilots_updateSystemFleet( const int deletedIndex );


/*
 * communication
 */
void pilot_message( Pilot *p, unsigned int target, const char *msg, int ignore_int );
void pilot_broadcast( Pilot *p, const char *msg, int ignore_int );
void pilot_distress( Pilot *p, const char *msg, int ignore_int );


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


#endif /* PILOT_H */
