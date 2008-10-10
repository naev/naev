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


#define PLAYER_ID       1 /**< Player pilot ID. */


/* Hyperspace parameters. */
#define HYPERSPACE_ENGINE_DELAY  3000 /**< time to warm up engine */
#define HYPERSPACE_FLY_DELAY     5000 /**< time it takes to hyperspace */
#define HYPERSPACE_STARS_BLUR    2000 /**< how long the stars blur */
#define HYPERSPACE_STARS_LENGTH  1000 /**< length the stars blur to at max */
#define HYPERSPACE_FADEOUT       1000 /**< how long the fade is */
#define HYPERSPACE_FUEL          100  /**< how much fuel it takes */
#define HYPERSPACE_THRUST        2000./**< How much thrust you use in hyperspace. */
#define HYPERSPACE_VEL           HYPERSPACE_THRUST*(HYPERSPACE_FLY_DELAY/1000) /**< Velocity at hyperspace. */
#define HYPERSPACE_ENTER_MIN     HYPERSPACE_VEL*0.5 /**< Minimum entering distance. */
#define HYPERSPACE_ENTER_MAX     HYPERSPACE_VEL*0.6 /**< Maxmimu entering distance. */
#define HYPERSPACE_EXIT_MIN      1500. /**< Minimum distance to begin jumping. */


#define PILOT_SIZE_APROX      0.8   /**, aproximation for pilot size */
#define PILOT_DISABLED_ARMOR  0.3   /**< armour % that gets it disabled */

/* hooks */
#define PILOT_HOOKS        4 /**< Max number of hooks a pilot can have. */
#define PILOT_HOOK_NONE    0 /**< No hook. */
#define PILOT_HOOK_DEATH   1 /**< Pilot died. */
#define PILOT_HOOK_BOARD   2 /**< Pilot got boarded. */
#define PILOT_HOOK_DISABLE 3 /**< Pilot got disabled. */
#define PILOT_HOOK_JUMP    4 /**< Pilot jumped. */


/* flags */
#define pilot_isFlag(p,f)  ((p)->flags & (f)) /**< Checks if flag f is set on pilot p. */
#define pilot_setFlag(p,f) ((p)->flags |= (f)) /**< Sets flag f on pilot p. */
#define pilot_rmFlag(p,f)  ((p)->flags ^= (f)) /**< Removes flag f on pilot p. */
/* creation */
#define PILOT_PLAYER       (1<<0) /**< Pilot is a player. */
#define PILOT_ESCORT       (1<<1) /**< Pilot is an escort. */
#define PILOT_CARRIED      (1<<2) /**< Pilot usually resides in a fighter bay. */
#define PILOT_EMPTY        (1<<5) /**< do not add pilot to stack. */
#define PILOT_NO_OUTFITS   (1<<6) /**< do not create the pilot with outfits. */
#define PILOT_HASTURRET    (1<<9) /**< pilot has turrets. */
#define PILOT_HASBEAMS     (1<<10) /**< Pilot has beam weapons. */
/* dynamic */
#define PILOT_HOSTILE      (1<<11) /**< pilot is hostile to the player. */
#define PILOT_COMBAT       (1<<12) /**< pilot is engaged in combat. */
#define PILOT_AFTERBURNER  (1<<13) /**< pilot has his afterburner activated. */
#define PILOT_HYP_PREP     (1<<15) /**< pilot is getting ready for hyperspace. */
#define PILOT_HYP_BEGIN    (1<<16) /**< pilot is starting engines. */
#define PILOT_HYPERSPACE   (1<<17) /**< pilot is in hyperspace. */
#define PILOT_BOARDED      (1<<18) /**< pilot has been boarded already. */
#define PILOT_DISABLED     (1<<19) /**< pilot is disabled. */
#define PILOT_DEAD         (1<<20) /**< pilot is in it's dying throes */
#define PILOT_DEATH_SOUND  (1<<21) /**< pilot just did death explosion. */
#define PILOT_EXPLODED     (1<<22) /**< pilot did final death explosion. */
#define PILOT_DELETE       (1<<25) /**< pilot will get deleted asap. */

/* makes life easier */
#define pilot_isPlayer(p)  ((p)->flags & PILOT_PLAYER) /**< Checks if pilot is a player. */
#define pilot_isDisabled(p) ((p)->flags & PILOT_DISABLED) /**< Checks if pilot is disabled. */


/**
 * @enum PilotOutfitState
 *
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
 * @struct PilotOutfit
 *
 * @brief Stores an outfit the pilot has.
 */
typedef struct PilotOutfit_ {
   Outfit* outfit; /**< Associated outfit. */
   int quantity; /**< Number of outfits of this type pilot has. */
   PilotOutfitState state; /**< State of the outfit. */
   int beamid; /**< ID of the beam used in this outfit, only used for beams. */
   unsigned int timer; /**< Used to store when it was last used. */
} PilotOutfit;


/**
 * @struct PilotCommodity
 *
 * @brief Stores a pilot commodity.
 */
typedef struct PilotCommodity_ {
   Commodity* commodity; /**< Assosciated commodity. */
   int quantity; /**< Amount player has. */
   unsigned int id; /**< Special mission id for cargo, 0 means none. */
} PilotCommodity;


/**
 * @struct Pilot
 *
 * @brief The representation of an in-game pilot.
 */
typedef struct Pilot_ {

   unsigned int id; /**< pilot's id, used for many functions */
   char* name; /**< pilot's name (if unique) */
   char* title; /**< title - usually indicating special properties - @todo use */

   int faction; /**< Pilot's faction. */

   /* Object caracteristics */
   Ship* ship; /**< ship pilot is flying */
   Solid* solid; /**< associated solid (physics) */
   int tsx; /**< current sprite x position., calculated on update. */
   int tsy; /**< current sprite y position, calculated on update. */

   /* Movement */
   double thrust; /**< Pilot's thrust. */
   double turn; /**< Pilot's turn. */
   double speed; /**< Pilot's speed. */

   /* Current health */
   double armour; /**< Current armour. */
   double shield; /**< Current shield. */
   double energy; /**< Current energy. */
   double fuel; /**< Current fuel. */
   double armour_max; /**< Maximum armour. */
   double shield_max; /**< Maximum shield. */
   double energy_max; /**< Maximum energy. */
   double fuel_max; /**< Maximum fuel. */
   double armour_regen; /**< Armour regeneration rate (per second). */
   double shield_regen; /**< Shield regeneration rate (per second). */
   double energy_regen; /**< Energy regeneration rate (per second). */

   /* Associated functions */
   void (*think)(struct Pilot_*); /**< AI thinking for the pilot */
   void (*update)(struct Pilot_*, const double); /**< updates the pilot */
   void (*render)(struct Pilot_*); /**< for rendering the pilot */

   /* Outfit management */
   PilotOutfit* outfits; /**< pilot outfit stack. */
   int noutfits; /**< pilot number of outfits. */
   PilotOutfit* secondary; /**< secondary weapon */
   PilotOutfit* ammo; /**< secondary ammo if needed */
   PilotOutfit* afterburner; /**< the afterburner */

   /* Jamming */
   double jam_range; /**< Range at which pilot starts jamming. */
   double jam_chance; /**< Jam chance. */

   /* Cargo */
   int credits; /**< monies the pilot has */
   PilotCommodity* commodities; /**< commodity and quantity */
   int ncommodities; /**< number of commodities. */
   int cargo_free; /**< Free commodity space. */

   /* Weapon properties */
   double weap_range; /**< Average range of primary weapons */
   double weap_speed; /**< Average speed of primary weapons */

   /* Misc */
   uint32_t flags; /**< used for AI and others */
   unsigned int ptimer; /**< generic timer for internal pilot use */
   int lockons; /**< Stores how many seeking weapons are targetting pilot */

   /* Hook attached to the pilot */
   int hook_type[PILOT_HOOKS]; /**< Type of the hook attached to the pilot. */
   int hook[PILOT_HOOKS]; /**< Hook ID */

   /* Escort stuff. */
   unsigned int parent; /**< Pilot's parent. */
   unsigned int *escorts; /**< Pilot's escorts. */
   int nescorts; /**< Number of pilot escorts. */

   /* AI */
   unsigned int target; /**< AI target. */
   AI_Profile* ai; /**< ai personality profile */
   unsigned int tcontrol; /**< timer for control tick */
   unsigned int timer[MAX_AI_TIMERS]; /**< timers for AI */
   Task* task; /**< current action */
} Pilot;


/**
 * @struct FleetPilot
 *
 * @brief Represents a pilot in a fleet.
 *
 * @sa Fleet
 * @sa Pilot
 */   
typedef struct FleetPilot_ {
   Ship* ship; /**< ship the pilot is flying */
   char* name; /**< used if they have a special name like uniques */
   int chance; /**< chance of this pilot appearing in the leet */
   char *ai; /**< ai different of fleet's global ai */
} FleetPilot;

/**
 * @struct Fleet
 *
 * @brief Represents a fleet.
 *
 * Fleets are used to create pilots, both from being in a system and from
 *  mission triggers.
 *
 * @sa FleetPilot
 */
typedef struct Fleet_ {
   char* name; /**< fleet name, used as the identifier */
   int faction; /**< faction of the fleet */

   char *ai; /**< AI profile to use */

   FleetPilot* pilots; /**< the pilots in the fleet */
   int npilots; /**< total number of pilots */
} Fleet;


/*
 * getting pilot stuff
 */
extern Pilot* player; /* the player */
Pilot* pilot_get( const unsigned int id );
unsigned int pilot_getNextID( const unsigned int id );
unsigned int pilot_getPrevID( const unsigned int id );
unsigned int pilot_getNearestEnemy( const Pilot* p );
unsigned int pilot_getNearestHostile (void); /* only for the player */
unsigned int pilot_getNearestPilot( const Pilot* p );
Fleet* fleet_get( const char* name );
int pilot_getJumps( const Pilot* p );


/*
 * misc
 */
void pilot_shoot( Pilot* p, const int secondary );
void pilot_shootStop( Pilot* p, const int secondary );
void pilot_hit( Pilot* p, const Solid* w, const unsigned int shooter,
      const DamageType dtype, const double damage );
double pilot_face( Pilot* p, const double dir );
void pilot_hyperspaceAbort( Pilot* p );
/* special outfit stuff. */
void pilot_switchSecondary( Pilot* p, int i );
void pilot_setSecondary( Pilot* p, const char* secondary );
void pilot_setAmmo( Pilot* p );
int pilot_getAmmo( Pilot* p, Outfit* o );
void pilot_setAfterburner( Pilot* p );
/* Escort stuff. */
int pilot_dock( Pilot *p, Pilot *target );
/* explosion. */
void pilot_explode( double x, double y, double radius,
      DamageType dtype, double damage, unsigned int parent );
/* outfits */
int pilot_freeSpace( Pilot* p ); /* weapon space */
int pilot_addOutfit( Pilot* pilot, Outfit* outfit, int quantity );
int pilot_rmOutfit( Pilot* pilot, Outfit* outfit, int quantity );
char* pilot_getOutfits( Pilot* pilot );
void pilot_calcStats( Pilot* pilot );
int pilot_oquantity( Pilot* p, PilotOutfit* w );
/* normal cargo */
int pilot_cargoUsed( Pilot* pilot ); /* gets how much cargo it has onboard */
int pilot_cargoFree( Pilot* p ); /* cargo space */
int pilot_addCargo( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_rmCargo( Pilot* pilot, Commodity* cargo, int quantity );
/* mission cargo - not to be confused with normal cargo */
unsigned int pilot_addMissionCargo( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_rmMissionCargo( Pilot* pilot, unsigned int cargo_id );


/*
 * creation
 */
void pilot_init( Pilot* dest, Ship* ship, char* name, int faction, char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel, const int flags );
unsigned int pilot_create( Ship* ship, char* name, int faction, char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel, const int flags );
Pilot* pilot_createEmpty( Ship* ship, char* name,
      int faction, char *ai, const int flags );
Pilot* pilot_copy( Pilot* src );


/*
 * init/cleanup
 */
void pilot_destroy(Pilot* p);
void pilots_free (void);
void pilots_clean (void);
void pilots_cleanAll (void);
void pilot_free( Pilot* p );
int fleet_load (void);
void fleet_free (void);


/*
 * update
 */
void pilots_update( double dt );
void pilots_render (void);


/*
 * hooks
 */
void pilot_addHook( Pilot *pilot, int type, int hook );
void pilot_runHook( Pilot* p, int hook_type );


#endif /* PILOT_H */
