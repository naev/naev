/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space_fdecl.h"

#include "collision.h"
#include "commodity.h"
#include "damagetype.h"
#include "nxml.h"
#include "opengl.h"
#include "physics.h"
#include "quadtree.h"

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
enum {
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
};

/**
 * @brief Represents a potential reward from the asteroid.
 */
typedef struct AsteroidReward_ {
   Commodity *material; /**< Material dropped. */
   int        quantity; /**< Maximum amount. */
   int        rarity;   /**< Rarity. */
} AsteroidReward;

/**
 * @brief Represents a type of asteroid.
 */
typedef struct AsteroidType_ {
   char           *name;        /**< Name of the asteroid type. */
   char           *scanned_msg; /**< Scanned message. */
   glTexture     **gfxs;        /**< asteroid possible gfxs. */
   CollPoly       *polygon;     /**< Collision polygons associated to gfxs. */
   AsteroidReward *material;    /**< Materials contained in the asteroid. */
   double          armour_min;  /**< Minimum "armour" of the asteroid. */
   double          armour_max;  /**< Maximum "armour" of the asteroid. */
   double          absorb;      /**< Absorption of the asteroid. */
   double          damage;      /**< Damage on explosion. */
   double          disable;     /**< Disable on explosion. */
   double          penetration; /**< Penetration of the explosion. */
   double          exp_radius;  /**< Explosion radius. */
   double          alert_range; /**< Range to alert other ships. */
} AsteroidType;

/**
 * @brief Represents a group of asteroids.
 */
typedef struct AsteroidTypeGroup_ {
   char          *name;    /**< Name of the type group. */
   AsteroidType **types;   /**< Types of asteroids in the group. */
   double        *weights; /**< Weights of each element in the group. */
   double         wtotal;  /**< Sum of weights in the group. */
} AsteroidTypeGroup;

/**
 * @brief Represents a single asteroid.
 */
typedef struct Asteroid_ {
   /* Intrinsics. */
   int                 id;      /**< ID of the asteroid, for targeting. */
   int                 parent;  /**< ID of the anchor parent. */
   int                 state;   /**< State of the asteroid. */
   const AsteroidType *type;    /**< Type of the asteroid. */
   const glTexture    *gfx;     /**< Graphic of the asteroid. */
   CollPoly           *polygon; /**< Collision polygon associated to gfx. */
   double              armour;  /**< Current "armour" of the asteroid. */
   /* Movement. */
   Solid  sol;  /**< Solid. */
   double ang;  /**< Angle. */
   double spin; /**< Spin. */
   /* Stats. */
   double timer;      /**< Internal timer for animations. */
   double timer_max;  /**< Internal timer initial value. */
   double scan_alpha; /**< Alpha value for scanning stuff. */
   int    scanned;    /**< Wether the player already scanned this asteroid. */
} Asteroid;

/**
 * @brief Represents an asteroid field anchor.
 */
typedef struct AsteroidAnchor_ {
   char               *label;     /**< Label used for unidiffs. */
   int                 id;        /**< ID of the anchor, for targeting. */
   vec2                pos;       /**< Position in the system (from center). */
   double              density;   /**< Density of the field. */
   Asteroid           *asteroids; /**< Asteroids belonging to the field. */
   int                 nmax;      /**< Maximum number of asteroids. */
   double              radius;    /**< Radius of the anchor. */
   double              area;      /**< Field's area. */
   AsteroidTypeGroup **groups;    /**< Groups of asteroids. */
   double             *groupsw;   /**< Weight of the groups of asteroids. */
   double              groupswtotal; /**< Sum of the weights of the groups. */
   double maxspeed; /**< Maxmimum speed the asteroids can have in the field. */
   double maxspin;  /**< Maxmimum spin the asteroids can have in the field. */
   double accel;    /**< Accel applied when out of radius towards center. */
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
   vec2   pos;     /**< Position in the system (from center). */
   double radius;  /**< Radius of the exclusion zone. */
   int    affects; /**< Temporary internal value when rendering. */
} AsteroidExclusion;

/* Initialization and parsing. */
int  asteroids_load( void );
void asteroids_free( void );
void asteroid_free( AsteroidAnchor *ast );
void asteroids_init( void );

/* Updating and rendering. */
void asteroids_update( double dt );
void asteroids_render( void );
void asteroids_renderOverlay( void );

/* Asteroid types. */
const AsteroidType *asttype_getAll( void );
AsteroidType       *asttype_getName( const char *name );

/* Asteroid type groups. */
const AsteroidTypeGroup *astgroup_getAll( void );
AsteroidTypeGroup       *astgroup_getName( const char *name );

/* Misc functions. */
int  asteroids_inField( const vec2 *p );
void asteroids_computeInternals( AsteroidAnchor *a );
void asteroid_hit( Asteroid *a, const Damage *dmg, int max_rarity,
                   double mine_bonus );
void asteroid_explode( Asteroid *a, int max_rarity, double mine_bonus );
void asteroid_collideQueryIL( AsteroidAnchor *anc, IntList *il, int x1, int y1,
                              int x2, int y2 );
