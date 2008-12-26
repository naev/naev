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


/*
 * enums
 */
typedef enum RadarShape_ { RADAR_RECT, RADAR_CIRCLE
} RadarShape; /**< Player's radar shape. */


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
int gui_init (void);
void gui_free (void);
void player_render (void);
void player_renderBG (void); /* renders BG layer player stuff */
void player_renderGUI( double dt ); /* renders GUI stuff */


/*
 * misc
 */
void player_message( const char *fmt, ... );
void player_clear (void);
void player_warp( const double x, const double y );
const char* player_rating (void);
/* Sounds. */
void player_playSound( int sound, int once );
void player_stopSound (void);
void player_soundPause (void);
void player_soundResume (void);
/* cargo */
int player_outfitOwned( const char* outfitname );
int player_cargoOwned( const char* commodityname );


/*
 * pilot ships
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
 * player missions
 */
void player_missionFinished( int id );
int player_missionAlreadyDone( int id );

/*
 * licenses
 */
void player_addLicense( char *license );
int player_hasLicense( char *license );
char **player_getLicenses( int *nlicenses );

/* 
 * keybind actions
 */
void player_targetHostile (void);
void player_targetNext (void);
void player_targetPrev (void);
void player_targetNearest (void);
void player_setRadarRel( int mod );
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


#endif /* PLAYER_H */
