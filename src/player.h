/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef PLAYER_H
#  define PLAYER_H


#include "pilot.h"


/* flag defines */
#define PLAYER_TURN_LEFT   (1<<0)   /* player is turning left */
#define PLAYER_TURN_RIGHT  (1<<1)   /* player is turning right */
#define PLAYER_REVERSE		(1<<2)	/* player is facing opposite of vel */
#define PLAYER_AFTERBURNER	(1<<3)	/* player is afterburning */
#define PLAYER_DESTROYED	(1<<9)	/* player is destroyed */
#define PLAYER_FACE        (1<<10)	/* player is facing target */
#define PLAYER_PRIMARY     (1<<11)	/* player is shooting primary weapon */
#define PLAYER_SECONDARY   (1<<12)	/* player is shooting secondary weapon */
#define PLAYER_LANDACK		(1<<13)	/* player has permission to land */
/* flag functions */
#define player_isFlag(f)   (player_flags & f)
#define player_setFlag(f)  if (!player_isFlag(f)) player_flags |= f
#define player_rmFlag(f)   if (player_isFlag(f)) player_flags ^= f


/*
 * the player
 */
extern Pilot* player;
extern char* player_name;
extern unsigned int player_flags;
extern int player_credits;
extern int player_crating;


/*
 * enums
 */
typedef enum RadarShape_ { RADAR_RECT, RADAR_CIRCLE
} RadarShape; /* for rendering fucntions */


/*
 * creation
 */
void player_new (void);
void player_newShip( Ship* ship );

/*
 * render
 */
int gui_init (void);
void gui_free (void);
void player_render (void);
void player_renderBG (void); /* renders BG layer player stuff */

/*
 * misc
 */
void player_message( const char *fmt, ... );
void player_clear (void);
void player_warp( const double x, const double y );
const char* player_rating (void);
int player_freeSpace (void);
int player_outfitOwned( const char* outfitname );
int player_cargoOwned( const char* commodityname );

/* 
 * keybind actions
 */
void player_setRadarRel( int mod );
void player_secondaryNext (void);
void player_targetPlanet (void);
void player_land (void);
void player_targetHyperspace (void);
void player_jump (void);
void player_screenshot (void);
void player_afterburn (void);
void player_afterburnOver (void);


#endif /* PLAYER_H */
