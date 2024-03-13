/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

extern int    paused;
extern int    player_paused;
extern double dt_mod;

void pause_setSpeed( double mod );

void pause_game( void );
void unpause_game( void );
void pause_player( void );
