/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include <stdint.h>

#include "colour.h"
#include "nlua.h"
#include "opengl_tex.h"
#include "space_fdecl.h"

// typedef void *FactionRef;
typedef int64_t FactionRef;

// #define FACTION_NULL ( ( FactionRef ) - 1 )
#define FACTION_NULL ( ( 1ll << 32ll ) + ( 1ll << 32ll ) - 1ll )

FactionRef faction_player( void );

#define FACTION_PLAYER                                                         \
   faction_player()        /**< Hardcoded player faction identifier. */
#define FACTION_LOGO_SM 64 /**< Size of "small" logo. */

typedef struct FactionGenerator_ {
   FactionRef id;     /**< Id of the generator. */
   double     weight; /**< Weight modifier. */
} FactionGenerator;

/* Get stuff */
FactionRef              faction_null( void );
int                     faction_isFaction( FactionRef f );
FactionRef              faction_exists( const char *name );
FactionRef              faction_get( const char *name );
FactionRef             *faction_getAll( void );
FactionRef             *faction_getAllVisible( void );
FactionRef             *faction_getKnown();
int                     faction_isStatic( FactionRef id );
int                     faction_isInvisible( FactionRef id );
int                     faction_setInvisible( FactionRef id, int state );
int                     faction_isKnown( FactionRef id );
int                     faction_isDynamic( FactionRef id );
const char             *faction_name( FactionRef f );
const char             *faction_shortname( FactionRef f );
const char             *faction_longname( FactionRef f );
const char             *faction_mapname( FactionRef f );
const char             *faction_description( FactionRef f );
const char             *faction_default_ai( FactionRef f );
const char *const      *faction_tags( FactionRef f );
double                  faction_lane_length_per_presence( FactionRef f );
double                  faction_lane_base_cost( FactionRef f );
void                    faction_clearEnemy( FactionRef f );
void                    faction_addEnemy( FactionRef f, FactionRef o );
void                    faction_rmEnemy( FactionRef f, FactionRef o );
void                    faction_clearAlly( FactionRef f );
void                    faction_addAlly( FactionRef f, FactionRef o );
void                    faction_rmAlly( FactionRef f, FactionRef o );
void                    faction_addNeutral( FactionRef f, FactionRef o );
void                    faction_rmNeutral( FactionRef f, FactionRef o );
const nlua_env         *faction_getScheduler( FactionRef f );
const nlua_env         *faction_getEquipper( FactionRef f );
const glTexture        *faction_logo( FactionRef f );
const glColour         *faction_colour( FactionRef f );
const FactionRef       *faction_getEnemies( FactionRef f );
const FactionRef       *faction_getAllies( FactionRef f );
FactionRef             *faction_getGroup( int which, const StarSystem *sys );
int                     faction_usesHiddenJumps( FactionRef f );
const FactionGenerator *faction_generators( FactionRef f );

/* Set stuff */
int    faction_setKnown( FactionRef id, int state );
double faction_reputationOverride( FactionRef f, int *set );
void   faction_setReputationOverride( FactionRef f, int set, double value );

/* player stuff */
double faction_hit( FactionRef f, const StarSystem *sys, double mod,
                    const char *source, int single );
double faction_hitTest( FactionRef f, const StarSystem *sys, double mod,
                        const char *source );
void   faction_modPlayer( FactionRef f, double mod, const char *source );
void   faction_modPlayerSingle( FactionRef f, double mod, const char *source );
void   faction_modPlayerRaw( FactionRef f, double mod );
void   faction_setReputation( FactionRef f, double value );
double faction_reputation( FactionRef f );
double faction_reputationDefault( FactionRef f );
int    faction_isPlayerFriend( FactionRef f );
int    faction_isPlayerEnemy( FactionRef f );
int    faction_isPlayerFriendSystem( FactionRef f, const StarSystem *sys );
int    faction_isPlayerEnemySystem( FactionRef f, const StarSystem *sys );
const char *faction_getStandingText( FactionRef f );
const char *faction_getStandingTextAtValue( FactionRef f, double value );
const char *faction_getStandingBroad( FactionRef f, int bribed, int override );
double      faction_reputationMax( FactionRef f );
const glColour *faction_reputationColour( FactionRef f );
char            faction_reputationColourChar( FactionRef f );
const glColour *faction_reputationColourSystem( FactionRef        f,
                                                const StarSystem *sys );
char faction_reputationColourCharSystem( FactionRef f, const StarSystem *sys );
void faction_applyLocalThreshold( FactionRef f, StarSystem *sys );
void faction_updateSingle( FactionRef f );
void faction_updateGlobal( void );

/* Works with only factions */
int areEnemies( FactionRef a, FactionRef b );
int areNeutral( FactionRef a, FactionRef b );
int areAllies( FactionRef a, FactionRef b );
int areEnemiesSystem( FactionRef a, FactionRef b, const StarSystem *sys );
int areAlliesSystem( FactionRef a, FactionRef b, const StarSystem *sys );

/* load/free */
int  factions_load( void );
int  factions_loadPost( void );
void factions_free( void );
void factions_reset( void );
void factions_resetLocal( void );
void factions_cleanLocal( void );
void faction_clearKnown( void );

/* Dynamic factions. */
void       factions_clearDynamic( void );
FactionRef faction_dynAdd( FactionRef base, const char *name,
                           const char *display, const char *ai,
                           const glColour *colour );
