/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space_fdecl.h" // IWYU pragma: keep

#include "collision.h"
#include "commodity.h"
#include "opengl.h"
#include "outfit.h"
#include "physics.h"
#include "quadtree.h"

#define ASTEROID_DEFAULT_RADIUS                                                \
   2500. /**< Default radius of an asteroid field. */
#define ASTEROID_DEFAULT_DENSITY                                               \
   1. /**< Default density of an asteroid field. */
#define ASTEROID_DEFAULT_MAXSPEED                                              \
   20. /**< Max speed of asteroids in an asteroid field. */
#define ASTEROID_DEFAULT_MAXSPIN                                               \
   M_PI / 5 /**< Max spin of asteroids in an asteroid field. */
#define ASTEROID_DEFAULT_ACCEL                                                 \
   1. /**< Acceleration applied when asteroid leaves asteroid field. */

#define ASTEROID_REF_AREA                                                      \
   250e3 /**< The "density" value in an asteroid field means 1 rock per this   \
            area. */

/* Asteroid status enum. Order is based on how asteroids are generated. */
typedef enum {
   ASTEROID_XX,       /**< Asteroid is not visible nor "exists". */
   ASTEROID_XX_TO_BG, /**< Asteroid is appearing into the background. */
   ASTEROID_XB, /**< Asteroid is in the background, coming from nothing and
                   going to foreground next. */
   ASTEROID_BG_TO_FG, /**< Asteroid is going from the background to the
                         foreground. */
   ASTEROID_FG,       /**< Asteroid is the foreground (interactive). */
   ASTEROID_FG_TO_BG, /**< Asteroid is going from the foreground to the
                         background. */
   ASTEROID_BX, /**< Asteroid is in the background, coming from foreground and
                   going to nothing next. */
   ASTEROID_BG_TO_XX,  /**< Asteroid is disappearing from the background. */
   ASTEROID_STATE_MAX, /**< Max amount of states. */
} AsteroidState;

typedef struct AsteroidType      AsteroidType;
typedef struct AsteroidTypeGroup AsteroidTypeGroup;
typedef struct Asteroid          Asteroid;
typedef struct Asteroid          AsteroidVec;
// typedef struct Asteroid*       AsteroidRef;
// #define ASTEROID_NULL   NULL
typedef int AsteroidRef;
#define ASTEROID_NULL -1

/**
 * @brief Represents an asteroid field anchor.
 */
typedef struct AsteroidAnchor_ {
   char               *label;     /**< Label used for unidiffs. */
   int                 id;        /**< ID of the anchor, for targeting. */
   vec2                pos;       /**< Position in the system (from centre). */
   double              density;   /**< Density of the field. */
   AsteroidVec        *asteroids; /**< Asteroids belonging to the field. */
   int                 nmax;      /**< Maximum number of asteroids. */
   double              radius;    /**< Radius of the anchor. */
   double              area;      /**< Field's area. */
   AsteroidTypeGroup **groups;    /**< Groups of asteroids. */
   double             *groupsw;   /**< Weight of the groups of asteroids. */
   double              groupswtotal; /**< Sum of the weights of the groups. */
   double maxspeed; /**< Maxmimum speed the asteroids can have in the field. */
   double maxspin;  /**< Maxmimum spin the asteroids can have in the field. */
   double accel;    /**< Accel applied when out of radius towards centre. */
   double margin; /**< Extra margin to use when doing distance computations. */
   /* Collision stuff. */
   Quadtree qt;      /**< Handles collisions. */
   int      qt_init; /**< Whether or not the quadtree has been initialized. */
   int      has_exclusion; /**< Used for updating. */
} AsteroidAnchor;

/**
 * @brief Represents an asteroid exclusion zone.
 */
typedef struct AsteroidExclusion_ {
   char  *label;   /**< Label used for unidiffs. */
   vec2   pos;     /**< Position in the system (from centre). */
   double radius;  /**< Radius of the exclusion zone. */
   int    affects; /**< Temporary internal value when rendering. */
} AsteroidExclusion;

/* Initialization and parsing. */
int  asteroids_load( void );
void asteroids_free( void );
void asteroid_initAnchor( AsteroidAnchor *ast );
void asteroid_freeAnchor( AsteroidAnchor *ast );
void asteroid_freeExclude( AsteroidExclusion *exc );
void asteroids_init( void );

/* Updating and rendering. */
void asteroids_update( double dt );
void asteroids_render( void );
void asteroids_renderOverlay( void );

/* Asteroid types. */
const AsteroidType *asttype_getAll( void );
AsteroidType       *asttype_getName( const char *name );

/* Asteroid type groups. */
AsteroidTypeGroup **astgroup_getAll( void );
AsteroidTypeGroup  *astgroup_getName( const char *name );
const char         *astgroup_name( const AsteroidTypeGroup *ast );

/* Getters. */
const Asteroid  *ast_get( const AsteroidAnchor *anc, AsteroidRef id );
int              ast_id( const Asteroid *ast );
int              ast_parent( const Asteroid *ast );
AsteroidState    ast_state( const Asteroid *ast );
const Solid     *ast_solid( const Asteroid *ast );
const glTexture *ast_gfx( const Asteroid *ast );
double           ast_gfx_width( const Asteroid *ast );
int              ast_test_collide( const Asteroid *ast, const CollPolyView *at,
                                   const vec2 *ap, vec2 *crash );
int              ast_scanned( const Asteroid *ast );
void             ast_set_scanned( const Asteroid *ast, int set );
CollPolyView    *ast_poly( const Asteroid *ast );
AsteroidRef      asteroid_closestPilot( const AsteroidAnchor *anc, double x,
                                        double y, double *d );

/* Misc functions. */
int  asteroids_inField( const vec2 *p );
int  asteroids_hasCommodity( const AsteroidAnchor *ast, CommodityRef com );
void asteroids_computeInternals( AsteroidAnchor *a );
void asteroid_hit( Asteroid *a, const Damage *dmg, int max_rarity,
                   double mine_bonus );
void asteroid_explode( Asteroid *a, int max_rarity, double mine_bonus );
void asteroid_collideQueryIL( AsteroidAnchor *anc, IntList *il, int x1, int y1,
                              int x2, int y2 );
