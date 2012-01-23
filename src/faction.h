/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef FACTION_H
#  define FACTION_H


#include "opengl.h"
#include "colour.h"
#include "nlua.h"


#define FACTION_PLAYER  0  /**< Hardcoded player faction identifier. */

#define FACTION_STATIC        (1<<0) /**< Faction doesn't change standing with player. */
#define FACTION_INVISIBLE     (1<<1) /**< Faction isn't exposed to the player. */
#define FACTION_KNOWN         (1<<2) /**< Faction is known to the player. */

#define faction_setFlag(fa,f) ((fa)->flags |= (f))
#define faction_rmFlag(fa,f)  ((fa)->flags &= ~(f))
#define faction_isFlag(fa,f)  ((fa)->flags & (f))
#define faction_isKnown(fa)   ((fa)->flags & (FACTION_KNOWN))

/**
 * @struct Faction
 *
 * @brief Represents a faction.
 */
typedef struct Faction_ {
   char *name; /**< Normal Name. */
   char *longname; /**< Long Name. */
   char *displayname; /**< Display name. */

   /* Graphics. */
   glTexture *logo_small; /**< Small logo. */
   glTexture *logo_tiny; /**< Tiny logo. */
   const glColour *colour; /**< Faction specific colour. */

   /* Enemies */
   int *enemies; /**< Enemies by ID of the faction. */
   int nenemies; /**< Number of enemies. */

   /* Allies */
   int *allies; /**< Allies by ID of the faction. */
   int nallies; /**< Number of allies. */

   /* Player information. */
   double player_def; /**< Default player standing. */
   double player; /**< Standing with player - from -100 to 100 */

   /* Scheduler. */
   lua_State *sched_state; /**< Lua scheduler script. */

   /* Behaviour. */
   lua_State *state; /**< Faction specific state. */

   /* Equipping. */
   lua_State *equip_state; /**< Faction equipper state. */


   /* Flags. */
   unsigned int flags; /**< Flags affecting the faction. */
} Faction;

/* get stuff */
int faction_isFaction( int f );
int faction_get( const char* name );
int* faction_getAll( int *n );
int* faction_getKnown( int *n );
int faction_isKnown( int id );
char* faction_name( int f );
char* faction_shortname( int f );
char* faction_longname( int f );
lua_State *faction_getScheduler( int f );
lua_State *faction_getEquipper( int f );
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
double faction_getPlayer( int f );
double faction_getPlayerDef( int f );
char* faction_getStanding( double mod );
char *faction_getStandingBroad( double mod );
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
