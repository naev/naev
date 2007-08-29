

#ifndef PLAYER_H
#  define PLAYER_H


#include "pilot.h"


/* flag defines */
#define PLAYER_TURN_LEFT   (1<<0)   /* player is turning left */
#define PLAYER_TURN_RIGHT  (1<<1)   /* player is turning right */
#define PLAYER_REVERSE		(1<<2)	/* player is facing opposite of vel */
#define PLAYER_FACE        (1<<10)   /* player is facing target */
#define PLAYER_PRIMARY     (1<<11)   /* player is shooting primary weapon */
#define PLAYER_SECONDARY   (1<<12)   /* player is shooting secondary weapon */
/* flag functions */
#define player_isFlag(f)   (player_flags & f)
#define player_setFlag(f)  (player_flags |= f)                            
#define player_rmFlag(f)   (player_flags ^= f) 


/*
 * the player
 */
extern Pilot* player;
extern unsigned int player_flags;
extern unsigned int credits;


/*
 * enums
 */
typedef enum { RADAR_RECT, RADAR_CIRCLE } RadarShape; /* for rendering fucntions */


/*
 * creation
 */
void player_new (void);

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

/* 
 * keybind actions
 */
void player_setRadarRel( int mod );
void player_board (void);
void player_secondaryNext (void);
void player_targetPlanet (void);
void player_land (void);
void player_targetHyperspace (void);
void player_jump (void);
void player_screenshot (void);


#endif /* PLAYER_H */
