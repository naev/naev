/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PLAYER_AUTONAV_H
#  define PLAYER_AUTONAV_H


#ifndef PLAYER_H
#error "Do not include player_autonav.h directly."
#endif /* PLAYER_H */


#define TIME_COMPRESSION_MAX     2500. /**< Maximum level of time compression (target speed to match). */


/* Autonav states. */
#define AUTONAV_JUMP_APPROACH   0 /**< Player is approaching a jump. */
#define AUTONAV_JUMP_BRAKE      1 /**< Player is braking at a jump. */
#define AUTONAV_POS_APPROACH   10 /**< Player is going to a position. */


void player_thinkAutonav( Pilot *pplayer );
void player_updateAutonav( double dt );
void player_autonavStart (void);
void player_autonavEnd (void);
void player_autonavAbort( const char *reason );
void player_autonavStartWindow( unsigned int wid, char *str);
void player_autonavPos( double x, double y );


#endif /* PLAYER_AUTONAV_H */
