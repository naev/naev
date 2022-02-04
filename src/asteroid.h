/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space_fdecl.h"

#include "commodity.h"
#include "opengl.h"
#include "physics.h"
#include "damagetype.h"
#include "nxml.h"

#define ASTEROID_REF_AREA     500e3 /**< The "density" value in an asteroid field means 1 rock per this area. */

/* Asteroid status enum. Order is based on how asteroids are generated. */
enum {
   ASTEROID_XX,      /**< Asteroid is not visible nor "exists". */
   ASTEROID_XX_TO_BG,/**< Asteroid is appearing into the background. */
   ASTEROID_XB,      /**< Asteroid is in the background, coming from nothing and going to foreground next. */
   ASTEROID_BG_TO_FG,/**< Asteroid is going from the background to the foreground. */
   ASTEROID_FG,      /**< Asteroid is the foreground (interactive). */
   ASTEROID_FG_TO_BG,/**< Asteroid is going from the foreground to the background. */
   ASTEROID_BX,      /**< Asteroid is in the background, coming from foreground and going to nothing next. */
   ASTEROID_BG_TO_XX,/**< Asteroid is disappearing from the background. */
   ASTEROID_STATE_MAX,/**< Max amount of states. */
};

/**
 * @brief Represents a type of asteroid.
 */
typedef struct AsteroidType_ {
   char *name;       /**< Name of the asteroid type. */
   glTexture **gfxs; /**< asteroid possible gfxs. */
   Commodity **material; /**< Materials contained in the asteroid. */
   int *quantity;    /**< Quantities of materials. */
   double armour;    /**< Starting "armour" of the asteroid. */
} AsteroidType;

/**
 * @brief Represents a small player-rendered debris.
 *
 * @TODO this should be moved to spfx and probably generated on the fly.
 */
typedef struct Debris_ {
   int gfxID;     /**< ID of the asteroid gfx. */
   Vector2d pos;  /**< Position. */
   Vector2d vel;  /**< Velocity. */
   double height; /**< height vs player */
} Debris;

/**
 * @brief Represents a single asteroid.
 */
typedef struct Asteroid_ {
   int id;        /**< ID of the asteroid, for targeting. */
   int parent;    /**< ID of the anchor parent. */
   int state;     /**< State of the asteroid. */
   Vector2d pos;  /**< Position. */
   Vector2d vel;  /**< Velocity. */
   int gfxID;     /**< ID of the asteroid gfx. */
   double timer;  /**< Internal timer for animations. */
   double timer_max; /**< Internal timer initial value. */
   int type;      /**< The ID of the asteroid type */
   int scanned;   /**< Wether the player already scanned this asteroid. */
   double armour; /**< Current "armour" of the asteroid. */
} Asteroid;

/**
 * @brief Represents an asteroid field anchor.
 */
typedef struct AsteroidAnchor_ {
   char *label;   /**< Label used for unidiffs. */
   int id;        /**< ID of the anchor, for targeting. */
   Vector2d pos;  /**< Position in the system (from center). */
   double density;/**< Density of the field. */
   Asteroid *asteroids; /**< Asteroids belonging to the field. */
   int nb;        /**< Number of asteroids. */
   Debris *debris;/**< Debris belonging to the field. */
   int ndebris;   /**< Number of debris. */
   double radius; /**< Radius of the anchor. */
   double area;   /**< Field's area. */
   int *type;     /**< Types of asteroids. */
   double maxspeed;/**< Maxmimum speed the asteroids can have in the field. */
   double thrust; /**< Thrust applied when out of radius towards center. */
   double margin; /**< Extra margin to use when doing distance computations. */
} AsteroidAnchor;

/**
 * @brief Represents an asteroid exclusion zone.
 */
typedef struct AsteroidExclusion_ {
   Vector2d pos;  /**< Position in the system (from center). */
   double radius; /**< Radius of the exclusion zone. */
   int affects;   /**< Temporary internal value when rendering. */
} AsteroidExclusion;

/* Initialization and parsing. */
int asteroids_load (void);
void asteroids_free (void);
void asteroids_parse( const xmlNodePtr parent, StarSystem *sys );
void asteroids_init (void);

/* Updating and rendering. */
void asteroids_update( double dt );
void asteroids_render (void);
void asteroids_renderOverlay (void);

/* Asteroid types. */
const AsteroidType *asttype_getAll (void);
const AsteroidType *asttype_get( int id );

/* Misc functions. */
int asteroids_inField( const Vector2d *p );
void asteroids_computeInternals( AsteroidAnchor *a );
void asteroid_hit( Asteroid *a, const Damage *dmg );
