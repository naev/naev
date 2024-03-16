/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include <time.h>

#include "nstring.h"
#include "pilot.h"

/** Player flag enum. */
enum {
   PLAYER_TURN_LEFT,  /**< player is turning left */
   PLAYER_TURN_RIGHT, /**< player is turning right */
   PLAYER_REVERSE,    /**< player is facing opposite of vel */
   PLAYER_ACCEL,      /**< player is accelerating */
   PLAYER_DESTROYED,  /**< player is destroyed */
   PLAYER_FACE,       /**< player is facing target */
   PLAYER_PRIMARY,    /**< player is shooting primary weapon */
   PLAYER_SECONDARY,  /**< player is shooting secondary weapon */
   PLAYER_LANDACK,    /**< player has permission to land */
   PLAYER_CREATING,   /**< player is being created */
   PLAYER_AUTONAV,    /**< player has autonavigation on. */
   PLAYER_NOLAND,     /**< player is not allowed to land (cleared on enter). */
   PLAYER_CINEMATICS, /**< Cinematics mode is enabled. */
   PLAYER_CINEMATICS_GUI, /**< Disable rendering the GUI when in cinematics
                             mode. */
   PLAYER_CINEMATICS_2X, /**< Disables usage of the 2x button when in cinematics
                            mode. */
   PLAYER_HOOK_LAND, /**< Hook hack to avoid running hooks in the middle of the
                        pilot stack. */
   PLAYER_HOOK_JUMPIN, /**< Hook hack to avoid running hooks in the middle of
                          the pilot stack. */
   PLAYER_HOOK_HYPER, /**< Hook hack to avoid runving hooks in the middle of the
                         pilot stack. */
   PLAYER_MFLY,       /**< Player has enabled mouse flying. */
   PLAYER_NOSAVE,     /**< Player is not allowed to save. */
   PLAYER_FLAGS_MAX   /**< Maximum number of flags. */
};

/** player_land() outcomes. */
enum {
   PLAYER_LAND_OK,     /**< landed successfully. */
   PLAYER_LAND_AGAIN,  /**< not yet close/slow enough to land. */
   PLAYER_LAND_DENIED, /**< cannot land here. */
};

typedef char PlayerFlags[PLAYER_FLAGS_MAX];

/* flag functions */
#define player_isFlag( f ) ( player.flags[f] )
#define player_setFlag( f ) ( player.flags[f] = 1 )
#define player_rmFlag( f ) ( player.flags[f] = 0 )

/* Control restoration reasons. */
enum {
   PINPUT_NULL,     /**< No specific reason. */
   PINPUT_MOVEMENT, /**< Player pressed a movement key. */
   PINPUT_AUTONAV,  /**< Player engaged autonav. */
   PINPUT_BRAKING   /**< Player engaged auto-braking. */
};

/**
 * @brief Wrapper for outfits.
 */
typedef struct PlayerOutfit_s {
   const Outfit *o; /**< Actual associated outfit. */
   int           q; /**< Amount of outfit owned. */
} PlayerOutfit_t;

/**
 * @brief Player ship.
 */
typedef struct PlayerShip_s {
   Pilot *p; /**< Pilot. */
   PilotWeaponSet
      weapon_sets[PILOT_WEAPON_SETS]; /**< All the weapon sets the pilot has. */
   int autoweap;                      /**< Automatically update weapon sets. */
   int favourite;                     /**< Whether or not it is favourited. */

   /* Fleet data. */
   int deployed; /**< Whether or not the ship is deployed. */

   /* Some meta-data. */
   double       time_played;      /**< Total time the player has used ship. */
   char        *acquired;         /**< How it got acquired. */
   time_t       acquired_date;    /**< When the player was created. */
   double       dmg_done_shield;  /**< Total damage done to shields. */
   double       dmg_done_armour;  /**< Total damage done to armour. */
   double       dmg_taken_shield; /**< Total damage taken to shields. */
   double       dmg_taken_armour; /**< Total damage taken to armour. */
   unsigned int ships_destroyed[SHIP_CLASS_TOTAL]; /**< Total number of ships
                                                      destroyed. */
   unsigned int jumped_times;  /**< Times the player jumped. */
   unsigned int landed_times;  /**< Times the player landed. */
   unsigned int death_counter; /**< Times the player got annihilated. */
} PlayerShip_t;

/**
 * The player struct.
 */
typedef struct Player_s {
   /* Player intrinsics. */
   Pilot       *p;    /**< Player's pilot. Same as ps.p. */
   PlayerShip_t ps;   /**< Player's ship with extra information. */
   char        *name; /**< Player's name. */

   /* Player data. */
   PlayerFlags flags;   /**< Player's flags. */
   int         autonav; /**< Current autonav state. */
   double      mousex;  /**< Mouse X position (for mouse flying). */
   double      mousey;  /**< Mouse Y position (for mouse flying). */
   double
         speed; /**< Gameplay speed modifier, multiplies the ship base speed. */
   char *chapter;      /**< Chapter information. */
   char *difficulty;   /**< Player-specific difficulty. */
   int   discover_off; /**< Player is unable to discover anything. */

   /* Loaded game version. */
   char *loaded_version; /**< Version of the loaded save game. */

   /* Options that we save. */
   char  *gui;           /**< Player's GUI. */
   double radar_res;     /**< Player's radar resolution. */
   int    eq_outfitMode; /**< Equipment outfit mode. */
   int    map_minimal;   /**< Map is set in minimal mode. */

   /* Player fleets. */
   int fleet_used;     /**< Currently used fleet capacity. */
   int fleet_capacity; /**< Total player fleet capacity. */

   /* Meta-data. */
   time_t       last_played;  /**< Date the save was last played. */
   double       time_played;  /**< Total time the player has played the game. */
   time_t       date_created; /**< When the player was created. */
   double       dmg_done_shield;  /**< Total damage done to shields. */
   double       dmg_done_armour;  /**< Total damage done to armour. */
   double       dmg_taken_shield; /**< Total damage taken to shields. */
   double       dmg_taken_armour; /**< Total damage taken to armour. */
   unsigned int ships_destroyed[SHIP_CLASS_TOTAL]; /**< Total number of ships
                                                      destroyed. */
   unsigned int jumped_times;  /**< Times the player jumped. */
   unsigned int landed_times;  /**< Times the player landed. */
   unsigned int death_counter; /**< Times the player got annihilated. */

   /* Meta-meta-data. */
   time_t time_since_save; /**< Time since last saved. */
} Player_t;

/*
 * Local player.
 */
extern Player_t player; /**< Local player. */

/*
 * Common player sounds.
 */
extern int snd_target;       /**< Sound when targeting. */
extern int snd_jump;         /**< Sound when can jump. */
extern int snd_nav;          /**< Sound when changing nav computer. */
extern int snd_hail;         /**< Hail sound. */
extern int snd_hypPowUp;     /**< Hyperspace power up sound. */
extern int snd_hypEng;       /**< Hyperspace engine sound. */
extern int snd_hypPowDown;   /**< Hyperspace power down sound. */
extern int snd_hypPowUpJump; /**< Hyperspace Power up to jump sound. */
extern int snd_hypJump;      /**< Hyperspace jump sound. */

/*
 * creation/cleanup
 */
int           player_init( void );
void          player_new( void );
PlayerShip_t *player_newShip( const Ship *ship, const char *def_name, int trade,
                              const char *acquired, int noname );
void          player_cleanup( void );

/*
 * Hook voodoo.
 */
void player_runHooks( void );

/*
 * Rendering.
 */
void player_renderUnderlay( double dt );
void player_render( double dt );

/*
 * Message stuff, in gui.c
 */
void player_messageToggle( int enable );
NONNULL( 1 ) PRINTF_FORMAT( 1, 2 ) void player_message( const char *fmt, ... );
NONNULL( 1 ) void player_messageRaw( const char *str );

/*
 * Misc.
 */
void      player_resetSpeed( void );
void      player_restoreControl( int reason, const char *str );
void      player_checkLandAck( void );
void      player_nolandMsg( const char *str );
void      player_clear( void );
void      player_warp( double x, double y );
int       player_hasCredits( credits_t amount );
credits_t player_modCredits( credits_t amount );
void      player_hailStart( void );
/* Sounds. */
void player_soundPlay( int sound, int once );
void player_soundPlayGUI( int sound, int once );
void player_soundStop( void );
void player_soundPause( void );
void player_soundResume( void );

/*
 * Player ships.
 */
int                 player_ships( char **sships, glTexture **tships );
void                player_shipsSort( void );
const PlayerShip_t *player_getShipStack( void );
int                 player_nships( void );
int                 player_hasShip( const char *shipname );
Pilot              *player_getShip( const char *shipname );
PlayerShip_t       *player_getPlayerShip( const char *shipname );
void                player_swapShip( const char *shipname, int move_cargo );
credits_t           player_shipPrice( const char *shipname, int count_unique );
void                player_rmShip( const char *shipname );
void                player_rmPlayerShip( PlayerShip_t *ps );

/*
 * Player outfits.
 */
int                   player_outfitOwned( const Outfit *o );
int                   player_outfitOwnedTotal( const Outfit *o );
const PlayerOutfit_t *player_getOutfits( void );
int                   player_getOutfitsFiltered( const Outfit ***outfits,
                                                 int ( *filter )( const Outfit *o ),
                                                 const char *name );
int                   player_numOutfits( void );
int                   player_addOutfit( const Outfit *o, int quantity );
int                   player_rmOutfit( const Outfit *o, int quantity );

/*
 * Player missions.
 */
void player_missionFinished( int id );
int  player_missionAlreadyDone( int id );
int *player_missionsDoneList(
   void ); /* Returns array that should not be freed. */

/*
 * Player events.
 */
void player_eventFinished( int id );
int  player_eventAlreadyDone( int id );
int *player_eventsDoneList(
   void ); /* Returns array that should not be freed. */

/*
 * Licenses.
 */
NONNULL( 1 ) void player_addLicense( const char *license );
int          player_hasLicense( const char *license );
const char **player_getLicenses( void );

/*
 * Escorts.
 */
int player_addEscorts( void );

/*
 * Pilot related stuff.
 */
void   player_dead( void );
void   player_destroyed( void );
void   player_think( Pilot *pplayer, const double dt );
void   player_update( Pilot *pplayer, const double dt );
void   player_updateSpecific( Pilot *pplayer, const double dt );
void   player_brokeHyperspace( void );
void   player_hyperspacePreempt( int );
int    player_getHypPreempt( void );
double player_dt_default( void );

/*
 * Targeting.
 */
/* Clearing. */
void player_targetClear( void );
void player_targetClearAll( void );
/* Spobs. */
void player_targetSpobSet( int id );
void player_targetSpob( void );
/* Asteroids. */
void player_targetAsteroidSet( int id_field, int id );
/* Hyperspace. */
void player_targetHyperspaceSet( int id, int autonavcont );
void player_targetHyperspace( void );
/* Pilots. */
void player_targetSet( unsigned int id );
void player_targetHostile( void );
void player_targetNext( int mode );
void player_targetPrev( int mode );
void player_targetNearest( void );
void player_targetEscort( int prev );

/*
 * Keybind actions.
 */
void player_weapSetPress( int id, double value, int repeat );
int  player_land( int loud );
void player_approach( void );
int  player_jump( void );
void player_screenshot( void );
void player_accel( double acc );
void player_accelOver( void );
void player_hail( void );
void player_hailSpob( void );
void player_autohail( void );
void player_toggleMouseFly( void );
void player_cooldownBrake( void );
void player_stealth( void );
