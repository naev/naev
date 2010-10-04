/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef FACTION_H
#  define FACTION_H


#include "opengl.h"
#include "colour.h"
#include "nlua.h"


#define FACTION_PLAYER  0  /**< Hardcoded player faction identifier. */


/* get stuff */
int faction_isFaction( int f );
int faction_get( const char* name );
int* faction_getAll( int *n );
char* faction_name( int f );
char* faction_shortname( int f );
char* faction_longname( int f );
lua_State *faction_getState( int f );
glTexture* faction_logoSmall( int f );
glTexture* faction_logoTiny( int f );
glColour* faction_colour( int f );
int* faction_getEnemies( int f, int *n );
int* faction_getAllies( int f, int *n );
int* faction_getGroup( int *n, int which );

/* player stuff */
void faction_modPlayer( int f, double mod );
void faction_modPlayerRaw( int f, double mod );
double faction_getPlayer( int f );
double faction_getPlayerDef( int f );
char* faction_getStanding( double mod );
char *faction_getStandingBroad( double mod );
glColour* faction_getColour( int f );
char faction_getColourChar( int f );

/* works with only factions */
int areEnemies( int a, int b );
int areAllies( int a, int b );

/* load/free */
int factions_load (void);
void factions_free (void);
void factions_reset (void);


#endif /* FACTION_H */
