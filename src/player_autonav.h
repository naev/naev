/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/* Autonav states. */
enum {
   AUTONAV_JUMP,  /**< Player is going to jump. */
   AUTONAV_POS,   /**< Player is going to a position. */
   AUTONAV_SPOB,  /**< Player is going to a spob. */
   AUTONAV_PILOT, /**< Player is going to a pilot. */
};

int player_autonavInit (void);

void player_thinkAutonav( Pilot *pplayer, double dt );
void player_updateAutonav( double dt );
void player_autonavResetSpeed (void);
void player_autonavStart (void);
void player_autonavEnd (void);
void player_autonavAbort( const char *reason );
void player_autonavStartWindow( unsigned int wid, const char *str );
void player_autonavPos( double x, double y );
void player_autonavSpob( const char *name, int tryland );
void player_autonavPil( unsigned int p );
void player_autonavBoard( unsigned int p );
void player_autonavReset( double s );
void player_autonavEnter (void);
