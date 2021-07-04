/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PLAYER_AUTONAV_H
#  define PLAYER_AUTONAV_H


#ifndef PLAYER_H
#error "Do not include player_autonav.h directly."
#endif /* PLAYER_H */


/* Autonav states. */
enum {
   AUTONAV_JUMP_APPROACH,  /**< Player is approaching a jump. */
   AUTONAV_JUMP_BRAKE,     /**< Player is braking at a jump. */
   AUTONAV_POS_APPROACH,   /**< Player is going to a position. */
   AUTONAV_PNT_APPROACH,   /**< Player is going to a planet. */
   AUTONAV_PNT_BRAKE,      /**< Player is braking at a planet. */
   AUTONAV_PLT_FOLLOW,     /**< Player is following a pilot. */
};


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
void player_autonavPil( unsigned int p );


#endif /* PLAYER_AUTONAV_H */
