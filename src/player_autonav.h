/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PLAYER_AUTONAV_H
#  define PLAYER_AUTONAV_H


#ifndef PLAYER_H
#error "Do not include player_autonav.h directly."
#endif /* PLAYER_H */


/* Autonav states. */
#define AUTONAV_JUMP_APPROACH   0 /**< Player is approaching a jump. */
#define AUTONAV_JUMP_BRAKE      1 /**< Player is braking at a jump. */
#define AUTONAV_POS_APPROACH   10 /**< Player is going to a position. */
#define AUTONAV_PNT_APPROACH   11 /**< Player is going to a planet. */


void player_thinkAutonav( Pilot *pplayer, double dt );
void player_updateAutonav( double dt );
void player_autonavResetSpeed (void);
void player_autonavStart (void);
void player_autonavEnd (void);
void player_autonavAbortJump( const char *reason );
void player_autonavAbort( const char *reason );
int player_autonavShouldResetSpeed (void);
void player_autonavStartWindow( unsigned int wid, char *str);
void player_autonavPos( double x, double y );
void player_autonavPnt( char *name );


#endif /* PLAYER_AUTONAV_H */
