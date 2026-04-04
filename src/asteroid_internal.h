#include "asteroid.h"

/**
 * @brief Represents a single asteroid.
 */
typedef struct Asteroid {
   /* Intrinsics. */
   int                 id;      /**< ID of the asteroid, for targeting. */
   int                 parent;  /**< ID of the anchor parent. */
   AsteroidState       state;   /**< State of the asteroid. */
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
