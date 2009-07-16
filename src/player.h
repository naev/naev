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
/* flag functions */
#define player_isFlag(f)   (player_flags & (f)) /**< Checks for a player flag. */
#define player_setFlag(f)  (player_flags |= (f)) /**< Sets a player flag. */ 
#define player_rmFlag(f)   (player_flags &= ~(f)) /**< Removes a player flag. */


/*
 * the player
 */
extern Pilot* player; /**< Player himself. */
extern char* player_name; /**< Player's name. */
extern unsigned int player_flags; /**< Player's flags. */
extern double player_crating; /**< Player's combat rating. */
extern int player_enemies; /**< Amount of enemies player has. */

/*
 * Targetting.
 */
extern int planet_target; /**< Targetted planet. -1 is none. */
extern int hyperspace_target; /**< Targetted hyperspace route. -1 is none. */


/*
 * Common player sounds.
 */
extern int snd_target; /**< Sound when targetting. */
extern int snd_jump; /**< Sound when can jump. */
extern int snd_nav; /**< Sound when changing nav computer. */
extern int snd_hypPowUp; /**< Hyperspace power up sound. */
extern int snd_hypEng; /**< Hyperspace engine sound. */
extern int snd_hypPowDown; /**< Hyperspace power down sound. */
extern int snd_hypPowUpJump; /**< Hyperspace Power up to jump sound. */
extern int snd_hypJump; /**< Hyperspace jump sound. */


/*
 * creation/cleanup
 */
void player_new (void);
int player_newShip( Ship* ship, double px, double py,
      double vx, double vy, double dir );
void player_cleanup (void);
int gui_load (const char* name);


/*
 * render
 */
void player_render( double dt );


/*
 * misc
 */
void player_messageRaw ( const char *str );
void player_message ( const char *fmt, ... );
void player_clear (void);
void player_warp( const double x, const double y );
const char* player_rating (void);
/* Sounds. */
void player_playSound( int sound, int once );
void player_stopSound (void);
void player_soundPause (void);
void player_soundResume (void);
/* cargo */
int player_cargoOwned( const char* commodityname );


/*
 * player ships
 */
void player_ships( char** sships, glTexture** tships );
int player_nships (void);
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
double player_faceHyperspace (void);


/* 
 * keybind actions
 */
void player_targetHostile (void);
void player_targetNext( int mode );
void player_targetPrev( int mode );
void player_targetNearest (void);
void player_targetEscort( int prev );
void player_secondaryNext (void);
void player_secondaryPrev (void);
void player_targetPlanet (void);
void player_land (void);
void player_targetHyperspace (void);
void player_jump (void);
void player_screenshot (void);
void player_afterburn (void);
void player_afterburnOver (void);
void player_accel( double acc );
void player_accelOver (void);
void player_startAutonav (void);
void player_abortAutonav( char *reason );
void player_hail (void);
void player_setFireMode( int mode );


#endif /* PLAYER_H */
