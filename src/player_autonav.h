/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/* Autonav states. */
enum {
   AUTONAV_JUMP_APPROACH,  /**< Player is approaching a jump. */
   AUTONAV_JUMP_BRAKE,     /**< Player is braking at a jump. */
   AUTONAV_POS_APPROACH,   /**< Player is going to a position. */
   AUTONAV_SPOB_APPROACH,   /**< Player is going to a spob. */
   AUTONAV_SPOB_LAND_APPROACH,/**< Player is going to land on a spob. */
   AUTONAV_SPOB_LAND_BRAKE, /**< Player is braking to land at a spob. */
   AUTONAV_PLT_FOLLOW,     /**< Player is following a pilot. */
   AUTONAV_PLT_BOARD_APPROACH,/**< Player is trying to board a pilot. */
   AUTONAV_PLT_BOARD_BRAKE,/**< Player is going to brake to board. */
};

int player_autonavInit (void);

void player_thinkAutonav( Pilot *pplayer, double dt );
void player_updateAutonav( double dt );
void player_autonavResetSpeed (void);
void player_autonavStart (void);
void player_autonavEnd (void);
void player_autonavAbortJump( const char *reason );
void player_autonavAbort( const char *reason );
int player_autonavShouldResetSpeed (void);
void player_autonavStartWindow( unsigned int wid, const char *str );
void player_autonavPos( double x, double y );
void player_autonavSpob( const char *name, int tryland );
void player_autonavPil( unsigned int p );
void player_autonavBoard( unsigned int p );
