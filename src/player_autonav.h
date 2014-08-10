/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
