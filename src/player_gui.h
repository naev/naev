/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/* Clean up. */
void player_guiCleanup( void );

/* Manipulation. */
int  player_guiAdd( const char *name );
void player_guiRm( const char *name );
int  player_guiCheck( const char *name );

/* High level. */
const char **player_guiList( void );
