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
#define HYPERSPACE_ENGINE_DELAY  3. /**< Time to warm up engine (seconds). */
#define HYPERSPACE_FLY_DELAY     5. /**< Time it takes to hyperspace (seconds). */
#define HYPERSPACE_STARS_BLUR    2. /**< How long the stars blur at max (pixels). */
#define HYPERSPACE_STARS_LENGTH  1000 /**< Length the stars blur to at max (seconds). */
#define HYPERSPACE_FADEOUT       1. /**< How long the fade is (seconds). */
#define HYPERSPACE_FUEL          100.  /**< how much fuel it takes */
#define HYPERSPACE_THRUST        2000./**< How much thrust you use in hyperspace. */
#define HYPERSPACE_VEL           HYPERSPACE_THRUST*HYPERSPACE_FLY_DELAY /**< Velocity at hyperspace. */
#define HYPERSPACE_ENTER_MIN     HYPERSPACE_VEL*0.5 /**< Minimum entering distance. */
#define HYPERSPACE_ENTER_MAX     HYPERSPACE_VEL*0.6 /**< Maxmimu entering distance. */
#define HYPERSPACE_EXIT_MIN      1500. /**< Minimum distance to begin jumping. */

#define PILOT_SIZE_APROX      0.8   /**< aproximation for pilot size */
#define PILOT_DISABLED_ARMOR  0.3   /**< armour % that gets it disabled */
#define PILOT_REFUEL_TIME     3. /**< Time to complete refueling. */
#define PILOT_REFUEL_RATE     HYPERSPACE_FUEL/PILOT_REFUEL_TIME /**< Fuel per second. */

/* hooks */
#define PILOT_HOOK_NONE    0 /**< No hook. */
#define PILOT_HOOK_DEATH   1 /**< Pilot died. */
#define PILOT_HOOK_BOARD   2 /**< Pilot got boarded. */
#define PILOT_HOOK_DISABLE 3 /**< Pilot got disabled. */
#define PILOT_HOOK_JUMP    4 /**< Pilot jumped. */


/* damage */
#define PILOT_HOSTILE_THRESHOLD  0.09 /**< Point at which pilot becomes hostile. */
#define PILOT_HOSTILE_DECAY      0.005 /**< Rate at which hostility decays. */


/* flags */
#define pilot_isFlag(p,f)  ((p)->flags & (f)) /**< Checks if flag f is set on pilot p. */
#define pilot_setFlag(p,f) ((p)->flags |= (f)) /**< Sets flag f on pilot p. */
#define pilot_rmFlag(p,f)  ((p)->flags &= ~(f)) /**< Removes flag f on pilot p. */
/* creation */
#define PILOT_PLAYER       (1<<0) /**< Pilot is a player. */
#define PILOT_ESCORT       (1<<1) /**< Pilot is an escort. */
#define PILOT_CARRIED      (1<<2) /**< Pilot usually resides in a fighter bay. */
#define PILOT_CREATED_AI   (1<<3) /** Pilot has already created AI. */
#define PILOT_EMPTY        (1<<5) /**< Do not add pilot to stack. */
#define PILOT_NO_OUTFITS   (1<<6) /**< Do not create the pilot with outfits. */
#define PILOT_HASTURRET    (1<<9) /**< Pilot has turrets. */
#define PILOT_HASBEAMS     (1<<10) /**< Pilot has beam weapons. */
/* dynamic */
#define PILOT_HOSTILE      (1<<11) /**< Pilot is hostile to the player. */
#define PILOT_FRIENDLY     (1<<12) /**< Pilot is friendly to the player. */
#define PILOT_COMBAT       (1<<13) /**< Pilot is engaged in combat. */
#define PILOT_AFTERBURNER  (1<<14) /**< Pilot has his afterburner activated. */
#define PILOT_HYP_PREP     (1<<15) /**< Pilot is getting ready for hyperspace. */
#define PILOT_HYP_BEGIN    (1<<16) /**< Pilot is starting engines. */
#define PILOT_HYPERSPACE   (1<<17) /**< Pilot is in hyperspace. */
#define PILOT_HYP_END      (1<<18) /**< Pilot is exiting hyperspace. */
#define PILOT_BOARDED      (1<<19) /**< Pilot has been boarded already. */
#define PILOT_BOARDING     (1<<20) /**< Pilot is currently boarding it's target. */
#define PILOT_BRIBED       (1<<21) /**< Pilot has been bribed already. */
#define PILOT_DISTRESSED   (1<<22) /**< Pilot has distressed once already. */
#define PILOT_REFUELING    (1<<23) /**< Pilot is trying to refueling. */
#define PILOT_REFUELBOARDING (1<<24) /**< Pilot is actively refueling. */
#define PILOT_DISABLED     (1<<26) /**< Pilot is disabled. */
#define PILOT_DEAD         (1<<27) /**< Pilot is in it's dying throes */
#define PILOT_DEATH_SOUND  (1<<28) /**< Pilot just did death explosion. */
#define PILOT_EXPLODED     (1<<29) /**< Pilot did final death explosion. */
#define PILOT_DELETE       (1<<30) /**< Pilot will get deleted asap. */

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
 * @brief Stores an outfit the pilot has.
 */
typedef struct PilotOutfit_ {
   Outfit* outfit; /**< Associated outfit. */
   int quantity; /**< Number of outfits of this type pilot has. */
   PilotOutfitState state; /**< State of the outfit. */
   int beamid; /**< ID of the beam used in this outfit, only used for beams. */
   int *mounts; /**< ID of each outfit mount. */
   int lastshot; /**< ID of the outfit that last shot. */
   double timer; /**< Used to store when it was last used. */
} PilotOutfit;


/**
 * @brief Stores a pilot commodity.
 */
typedef struct PilotCommodity_ {
   Commodity* commodity; /**< Assosciated commodity. */
   int quantity; /**< Amount player has. */
   unsigned int id; /**< Special mission id for cargo, 0 means none. */
} PilotCommodity;


/**
 * @brief A wrapper for pilot hooks.
 */
typedef struct PilotHook_ {
   int type; /**< Type of hook. */
   int id; /**< Hook ID assosciated with pilot hook. */
} PilotHook;


/**
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
   double fuel; /**< Current fuel. */
   double armour_max; /**< Maximum armour. */
   double shield_max; /**< Maximum shield. */
   double fuel_max; /**< Maximum fuel. */
   double armour_regen; /**< Armour regeneration rate (per second). */
   double shield_regen; /**< Shield regeneration rate (per second). */

   /* Energy is handled a bit differently. */
   double energy; /**< Current energy. */
   double energy_max; /**< Maximum energy. */
   double energy_regen; /**< Energy regeneration rate (per second). */
   double energy_tau; /**< Tau regeneration rate for energy. */

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
   unsigned int credits; /**< monies the pilot has */
   PilotCommodity* commodities; /**< commodity and quantity */
   int ncommodities; /**< number of commodities. */
   int cargo_free; /**< Free commodity space. */

   /* Weapon properties */
   double weap_range; /**< Average range of primary weapons */
   double weap_speed; /**< Average speed of primary weapons */

   /* Misc */
   uint32_t flags; /**< used for AI and others */
   double ptimer; /**< generic timer for internal pilot use */
   int lockons; /**< Stores how many seeking weapons are targetting pilot */
   int *mounted; /**< Number of mounted outfits on the mount. */
   double player_damage; /**< Accumulates damage done by player for hostileness.
                              In per one of max shield + armour. */

   /* Hook attached to the pilot */
   PilotHook *hooks; /**< Pilot hooks. */
   int nhooks; /**< Number of pilot hooks. */

   /* Escort stuff. */
   unsigned int parent; /**< Pilot's parent. */
   unsigned int *escorts; /**< Pilot's escorts. */
   int nescorts; /**< Number of pilot escorts. */

   /* AI */
   unsigned int target; /**< AI target. */
   AI_Profile* ai; /**< ai personality profile */
   double tcontrol; /**< timer for control tick */
   double timer[MAX_AI_TIMERS]; /**< timers for AI */
   Task* task; /**< current action */
} Pilot;


/*
 * getting pilot stuff
 */
extern Pilot* player; /* the player */
Pilot* pilot_get( const unsigned int id );
unsigned int pilot_getNextID( const unsigned int id, int mode );
unsigned int pilot_getPrevID( const unsigned int id, int mode );
unsigned int pilot_getNearestEnemy( const Pilot* p );
unsigned int pilot_getNearestHostile (void); /* only for the player */
unsigned int pilot_getNearestPilot( const Pilot* p );
int pilot_getJumps( const Pilot* p );


/*
 * Combat.
 */
void pilot_shoot( Pilot* p, int type );
void pilot_shootSecondary( Pilot* p );
void pilot_shootStop( Pilot* p, const int secondary );
double pilot_hit( Pilot* p, const Solid* w, const unsigned int shooter,
      const DamageType dtype, const double damage );
void pilot_explode( double x, double y, double radius,
      DamageType dtype, double damage, unsigned int parent );
double pilot_face( Pilot* p, const double dir );


/*
 * Outfits.
 */
/* Generic stuff. */
int pilot_freeSpace( Pilot* p ); /* weapon space */
int pilot_addOutfit( Pilot* pilot, Outfit* outfit, int quantity );
int pilot_rmOutfit( Pilot* pilot, Outfit* outfit, int quantity );
char* pilot_getOutfits( Pilot* pilot );
void pilot_calcStats( Pilot* pilot );
int pilot_oquantity( Pilot* p, PilotOutfit* w );
/* Special outfit stuff. */
int pilot_getMount( Pilot *p, int id, Vector2d *v );
void pilot_switchSecondary( Pilot* p, int i );
void pilot_setAmmo( Pilot* p );
int pilot_getAmmo( Pilot* p, Outfit* o );
void pilot_setAfterburner( Pilot* p );


/*
 * Cargo.
 */
/* Normal. */
int pilot_cargoUsed( Pilot* pilot ); /* gets how much cargo it has onboard */
int pilot_cargoFree( Pilot* p ); /* cargo space */
int pilot_addCargo( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_rmCargo( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_moveCargo( Pilot* dest, Pilot* src );
/* mission cargo - not to be confused with normal cargo */
unsigned int pilot_addMissionCargo( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_rmMissionCargo( Pilot* pilot, unsigned int cargo_id, int jettison );


/* Misc. */
int pilot_refuelStart( Pilot *p );
void pilot_hyperspaceAbort( Pilot* p );
void pilot_clearTimers( Pilot *pilot );
int pilot_dock( Pilot *p, Pilot *target );


/*
 * creation
 */
void pilot_init( Pilot* dest, Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const unsigned int flags );
unsigned int pilot_create( Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const unsigned int flags );
Pilot* pilot_createEmpty( Ship* ship, const char* name,
      int faction, const char *ai, const unsigned int flags );
Pilot* pilot_copy( Pilot* src );


/*
 * init/cleanup
 */
void pilot_destroy(Pilot* p);
void pilots_free (void);
void pilots_clean (void);
void pilots_cleanAll (void);
void pilot_free( Pilot* p );


/*
 * update
 */
void pilots_update( double dt );
void pilots_render (void);


/*
 * communication
 */
void pilot_message( Pilot *p, unsigned int target, const char *msg, int ignore_int );
void pilot_broadcast( Pilot *p, const char *msg, int ignore_int );
void pilot_distress( Pilot *p, const char *msg, int ignore_int );


/*
 * Sensors.
 */
void pilot_updateSensorRange (void);
int pilot_inRange( const Pilot *p, double x, double y );
int pilot_inRangePilot( const Pilot *p, const Pilot *target );
int pilot_inRangePlanet( const Pilot *p, int target );


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


/*
 * hooks
 */
void pilot_addHook( Pilot *pilot, int type, int hook );
void pilot_runHook( Pilot* p, int hook_type );


#endif /* PILOT_H */
