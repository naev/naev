/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PLAYER_H
#  define PLAYER_H


#include "pilot.h"


/* flag defines */
#define PLAYER_TURN_LEFT   (1<<0)   /**< player is turning left */
#define PLAYER_TURN_RIGHT  (1<<1)   /**< player is turning right */
#define PLAYER_REVERSE     (1<<2)   /**< player is facing opposite of vel */
#define PLAYER_AFTERBURNER (1<<3)   /**< player is afterburning */
#define PLAYER_DESTROYED   (1<<9)   /**< player is destroyed */
#define PLAYER_FACE        (1<<10)  /**< player is facing target */
#define PLAYER_PRIMARY     (1<<11)  /**< player is shooting primary weapon */
#define PLAYER_PRIMARY_L   (1<<12)  /**< player shot primary weapon last frame. */
#define PLAYER_SECONDARY   (1<<13)  /**< player is shooting secondary weapon */
#define PLAYER_SECONDARY_L (1<<14)  /**< player shot secondary last frame. */
#define PLAYER_LANDACK     (1<<15)  /**< player has permission to land */
#define PLAYER_CREATING    (1<<16)  /**< player is being created */
#define PLAYER_AUTONAV     (1<<17)  /**< player has autonavigation on. */
#define PLAYER_NOLAND      (1<<18)  /**< player is not allowed to land (cleared on enter). */
/* flag functions */
#define player_isFlag(f)   (player.flags & (f)) /**< Checks for a player flag. */
#define player_setFlag(f)  (player.flags |= (f)) /**< Sets a player flag. */
#define player_rmFlag(f)   (player.flags &= ~(f)) /**< Removes a player flag. */


/* Autonav states. */
#define AUTONAV_JUMP_APPROACH   0 /**< Player is approaching a jump. */
#define AUTONAV_JUMP_BRAKE      1 /**< Player is braking at a jump. */
#define AUTONAV_POS_APPROACH   10 /**< Player is going to a position. */


/**
 * The player struct.
 */
typedef struct Player_s {
   /* Player intrinsecs. */
   Pilot *p; /**< Player's pilot. */
   char *name; /**< Player's name. */
   char *gui; /**< Player's GUI. */
   int guiOverride;

   /* Player data. */
   unsigned int flags; /**< Player's flags. */
   int enemies; /**< Amount of enemies the player has. */
   double crating; /**< Combat rating. */
   int autonav; /**< Current autonav state. */
   Vector2d autonav_pos; /**< Target autonav position. */
} Player_t;


/*
 * Local player.
 */
extern Player_t player; /**< Local player. */


/*
 * Common player sounds.
 */
extern int snd_target; /**< Sound when targetting. */
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
void player_new (void);
int player_newShip( Ship* ship, const char *def_name, int trade );
void player_cleanup (void);


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
void player_clear (void);
void player_warp( const double x, const double y );
const char* player_rating (void);
int player_hasCredits( int amount );
unsigned long player_modCredits( int amount );
void player_hailStart (void);
/* Sounds. */
void player_playSound( int sound, int once );
void player_stopSound (void);
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
int player_shipPrice( char* shipname );
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

/*
 * keybind actions
 */
void player_weapSetPress( int id, int type );
void player_targetHostile (void);
void player_targetNext( int mode );
void player_targetPrev( int mode );
void player_targetNearest (void);
void player_targetEscort( int prev );
void player_targetClear (void);
void player_targetPlanet (void);
void player_land (void);
void player_targetHyperspace (void);
void player_jump (void);
void player_screenshot (void);
void player_afterburn (void);
void player_afterburnOver (void);
void player_accel( double acc );
void player_accelOver (void);
void player_autonavStart (void);
void player_autonavAbort( char *reason );
void player_autonavStartWindow( unsigned int wid, char *str);
void player_autonavPos( double x, double y );
void player_hail (void);
void player_autohail (void);


#endif /* PLAYER_H */
