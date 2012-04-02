/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PLAYER_H
#  define PLAYER_H


#include "pilot.h"

/* flag defines */
#define PLAYER_TURN_LEFT   0   /**< player is turning left */
#define PLAYER_TURN_RIGHT  1   /**< player is turning right */
#define PLAYER_REVERSE     2   /**< player is facing opposite of vel */
#define PLAYER_DESTROYED   9   /**< player is destroyed */
#define PLAYER_FACE        10  /**< player is facing target */
#define PLAYER_PRIMARY     11  /**< player is shooting primary weapon */
#define PLAYER_PRIMARY_L   12  /**< player shot primary weapon last frame. */
#define PLAYER_SECONDARY   13  /**< player is shooting secondary weapon */
#define PLAYER_SECONDARY_L 14  /**< player shot secondary last frame. */
#define PLAYER_LANDACK     15  /**< player has permission to land */
#define PLAYER_CREATING    16  /**< player is being created */
#define PLAYER_AUTONAV     17  /**< player has autonavigation on. */
#define PLAYER_NOLAND      18  /**< player is not allowed to land (cleared on enter). */
#define PLAYER_DOUBLESPEED 19  /**< player is running at double speed. */
#define PLAYER_CINEMATICS_GUI 20 /**< Disable rendering the GUI when in cinematics mode. */
#define PLAYER_CINEMATICS_2X 21 /**< Disables usage of the 2x button when in cinematics mode. */
#define PLAYER_HOOK_LAND   25
#define PLAYER_HOOK_JUMPIN 26
#define PLAYER_HOOK_HYPER  27
#define PLAYER_TUTORIAL    30  /**< Player is doing the tutorial. */
#define PLAYER_MFLY        31  /**< Player has enabled mouse flying. */
#define PLAYER_NOSAVE      32  /**< Player is not allowed to save. */
#define PLAYER_FLAGS_MAX   PLAYER_NOSAVE + 1 /* Maximum number of flags. */
typedef char PlayerFlags[ PLAYER_FLAGS_MAX ];

/* flag functions */
#define player_isFlag(f)   (player.flags[f])
#define player_setFlag(f)  (player.flags[f] = 1)
#define player_rmFlag(f)   (player.flags[f] = 0)

/* comfort flags. */
#define player_isTut()     player_isFlag(PLAYER_TUTORIAL)


#include "player_autonav.h"


/**
 * The player struct.
 */
typedef struct Player_s {
   /* Player intrinsics. */
   Pilot *p; /**< Player's pilot. */
   char *name; /**< Player's name. */
   char *gui; /**< Player's GUI. */
   int guiOverride; /**< GUI is overridden (not default). */

   /* Player data. */
   PlayerFlags flags; /**< Player's flags. */
   int enemies; /**< Amount of enemies the player has. */
   double crating; /**< Combat rating. */
   int autonav; /**< Current autonav state. */
   Vector2d autonav_pos; /**< Target autonav position. */
   char *autonavmsg; /**< String to print on arrival. */
   double tc_max; /**< Maximum time compression value (bounded by ship speed or conf setting). */
   double autonav_timer; /**< Timer that begins counting down when autonav aborts due to combat. */
   double mousex; /**< Mouse X position (for mouse flying). */
   double mousey; /**< Mouse Y position (for mouse flying). */
} Player_t;


/*
 * Local player.
 */
extern Player_t player; /**< Local player. */


/*
 * Common player sounds.
 */
extern int snd_target; /**< Sound when targeting. */
extern int snd_jump; /**< Sound when can jump. */
extern int snd_nav; /**< Sound when changing nav computer. */
extern int snd_hail; /**< Hail sound. */
extern int snd_hypPowUp; /**< Hyperspace power up sound. */
extern int snd_hypEng; /**< Hyperspace engine sound. */
extern int snd_hypPowDown; /**< Hyperspace power down sound. */
extern int snd_hypPowUpJump; /**< Hyperspace Power up to jump sound. */
extern int snd_hypJump; /**< Hyperspace jump sound. */


/*
 * creation/cleanup
 */
int player_init (void);
void player_new (void);
void player_newTutorial (void);
Pilot* player_newShip( Ship* ship, const char *def_name,
      int trade, int noname );
void player_cleanup (void);

/*
 * Hook voodoo.
 */
void player_runHooks (void);


/*
 * render
 */
void player_render( double dt );


/*
 * Message stuff, in gui.c
 */
void player_messageToggle( int enable );
void player_message( const char *fmt, ... );
void player_messageRaw ( const char *str );

/*
 * misc
 */
void player_nolandMsg( const char *str );
void player_clear (void);
void player_warp( const double x, const double y );
const char* player_rating (void);
int player_hasCredits( credits_t amount );
credits_t player_modCredits( credits_t amount );
void player_hailStart (void);
/* Sounds. */
void player_soundPlay( int sound, int once );
void player_soundPlayGUI( int sound, int once );
void player_soundStop (void);
void player_soundPause (void);
void player_soundResume (void);


/*
 * player ships
 */
void player_ships( char** sships, glTexture** tships );
int player_nships (void);
int player_hasShip( char* shipname );
Pilot* player_getShip( char* shipname );
char* player_getLoc( char* shipname );
void player_setLoc( char* shipname, char* loc );
void player_swapShip( char* shipname );
credits_t player_shipPrice( char* shipname );
void player_rmShip( char* shipname );


/*
 * player outfits.
 */
int player_outfitOwned( const Outfit *o );
void player_getOutfits( char** soutfits, glTexture** toutfits );
int player_numOutfits (void);
int player_addOutfit( const Outfit *o, int quantity );
int player_rmOutfit( const Outfit *o, int quantity );


/*
 * player missions
 */
void player_missionFinished( int id );
int player_missionAlreadyDone( int id );


/*
 * player events
 */
void player_eventFinished( int id );
int player_eventAlreadyDone( int id );


/*
 * licenses
 */
void player_addLicense( char *license );
int player_hasLicense( char *license );
char **player_getLicenses( int *nlicenses );


/*
 * escorts
 */
void player_clearEscorts (void);
int player_addEscorts (void);


/*
 * pilot related stuff
 */
void player_dead (void);
void player_destroyed (void);
void player_think( Pilot* pplayer, const double dt );
void player_update( Pilot *pplayer, const double dt );
void player_updateSpecific( Pilot *pplayer, const double dt );
void player_brokeHyperspace (void);
void player_hyperspacePreempt( int );
int player_getHypPreempt(void);

/*
 * Targeting.
 */
/* Clearing. */
void player_targetClear (void);
/* Planets. */
void player_targetPlanetSet( int id );
void player_targetPlanet (void);
/* Hyperspace. */
void player_targetHyperspaceSet( int id );
void player_targetHyperspace (void);
/* Pilots. */
void player_targetSet( unsigned int id );
void player_targetHostile (void);
void player_targetNext( int mode );
void player_targetPrev( int mode );
void player_targetNearest (void);
void player_targetEscort( int prev );

/*
 * keybind actions
 */
void player_weapSetPress( int id, int type, int repeat );
void player_land (void);
int player_jump (void);
void player_screenshot (void);
void player_accel( double acc );
void player_accelOver (void);
void player_hail (void);
void player_hailPlanet (void);
void player_autohail (void);
void player_toggleMouseFly(void);
void player_toggleCooldown(void);


#endif /* PLAYER_H */
