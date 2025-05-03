/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"
#include "nlua.h"
#include "opengl_tex.h"
#include "space_fdecl.h"

extern int faction_player;

#define FACTION_PLAYER                                                         \
   faction_player          /**< Hardcoded player faction identifier. */
#define FACTION_LOGO_SM 64 /**< Size of "small" logo. */

typedef struct FactionGenerator_ {
   int    id;     /**< Id of the generator. */
   double weight; /**< Weight modifier. */
} FactionGenerator;

/* Get stuff */
int                     faction_isFaction( int f );
int                     faction_exists( const char *name );
int                     faction_get( const char *name );
int                    *faction_getAll( void );
int                    *faction_getAllVisible( void );
int                    *faction_getKnown();
int                     faction_isStatic( int id );
int                     faction_isInvisible( int id );
int                     faction_setInvisible( int id, int state );
int                     faction_isKnown( int id );
int                     faction_isDynamic( int id );
const char             *faction_name( int f );
const char             *faction_shortname( int f );
const char             *faction_longname( int f );
const char             *faction_mapname( int f );
const char             *faction_description( int f );
const char             *faction_default_ai( int f );
const char *const      *faction_tags( int f );
double                  faction_lane_length_per_presence( int f );
double                  faction_lane_base_cost( int f );
void                    faction_clearEnemy( int f );
void                    faction_addEnemy( int f, int o );
void                    faction_rmEnemy( int f, int o );
void                    faction_clearAlly( int f );
void                    faction_addAlly( int f, int o );
void                    faction_rmAlly( int f, int o );
nlua_env               *faction_getScheduler( int f );
nlua_env               *faction_getEquipper( int f );
const glTexture        *faction_logo( int f );
const glColour         *faction_colour( int f );
const int              *faction_getEnemies( int f );
const int              *faction_getAllies( int f );
int                    *faction_getGroup( int which );
int                     faction_usesHiddenJumps( int f );
const FactionGenerator *faction_generators( int f );

/* Set stuff */
int    faction_setKnown( int id, int state );
double faction_reputationOverride( int f, int *set );
void   faction_setReputationOverride( int f, int set, double value );

/* player stuff */
double      faction_hit( int f, const StarSystem *sys, double mod,
                         const char *source, int single );
double      faction_hitTest( int f, const StarSystem *sys, double mod,
                             const char *source );
void        faction_modPlayer( int f, double mod, const char *source );
void        faction_modPlayerSingle( int f, double mod, const char *source );
void        faction_modPlayerRaw( int f, double mod );
void        faction_setReputation( int f, double value );
double      faction_reputation( int f );
double      faction_reputationDefault( int f );
int         faction_isPlayerFriend( int f );
int         faction_isPlayerEnemy( int f );
int         faction_isPlayerFriendSystem( int f, const StarSystem *sys );
int         faction_isPlayerEnemySystem( int f, const StarSystem *sys );
const char *faction_getStandingText( int f );
const char *faction_getStandingTextAtValue( int f, double value );
const char *faction_getStandingBroad( int f, int bribed, int override );
double      faction_reputationMax( int f );
const glColour *faction_reputationColour( int f );
char            faction_reputationColourChar( int f );
const glColour *faction_reputationColourSystem( int f, const StarSystem *sys );
char faction_reputationColourCharSystem( int f, const StarSystem *sys );
void faction_applyLocalThreshold( int f, StarSystem *sys );
void faction_updateSingle( int f );
void faction_updateGlobal( void );

/* Works with only factions */
int areEnemies( int a, int b );
int areNeutral( int a, int b );
int areAllies( int a, int b );
int areEnemiesSystem( int a, int b, const StarSystem *sys );
int areAlliesSystem( int a, int b, const StarSystem *sys );

/* load/free */
int  factions_load( void );
int  factions_loadPost( void );
void factions_free( void );
void factions_reset( void );
void factions_resetLocal( void );
void factions_cleanLocal( void );
void faction_clearKnown( void );

/* Dynamic factions. */
void factions_clearDynamic( void );
int  faction_dynAdd( int base, const char *name, const char *display,
                     const char *ai, const glColour *colour );
