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
int* faction_getKnown( int *n );
int faction_isInvisible( int id );
int faction_setInvisible( int id, int state );
int faction_isKnown( int id );
char* faction_name( int f );
char* faction_shortname( int f );
char* faction_longname( int f );
void faction_addEnemy( int f, int o);
void faction_rmEnemy( int f, int o);
void faction_addAlly( int f, int o);
void faction_rmAlly( int f, int o);
nlua_env faction_getScheduler( int f );
nlua_env faction_getEquipper( int f );
glTexture* faction_logoSmall( int f );
glTexture* faction_logoTiny( int f );
const glColour* faction_colour( int f );
int* faction_getEnemies( int f, int *n );
int* faction_getAllies( int f, int *n );
int* faction_getGroup( int *n, int which );

/* set stuff */
int faction_setKnown( int id, int state );

/* player stuff */
void faction_modPlayer( int f, double mod, const char *source );
void faction_modPlayerSingle( int f, double mod, const char *source );
void faction_modPlayerRaw( int f, double mod );
void faction_setPlayer( int f, double value );
double faction_getPlayer( int f );
double faction_getPlayerDef( int f );
int faction_isPlayerFriend( int f );
int faction_isPlayerEnemy( int f );
const char *faction_getStandingText( int f );
const char *faction_getStandingBroad( int f, int bribed, int override );
const glColour* faction_getColour( int f );
char faction_getColourChar( int f );

/* works with only factions */
int areEnemies( int a, int b );
int areAllies( int a, int b );

/* load/free */
int factions_load (void);
void factions_free (void);
void factions_reset (void);
void faction_clearKnown(void);


#endif /* FACTION_H */
