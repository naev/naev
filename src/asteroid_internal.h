#include "asteroid.h"

/**
 * @brief Represents a potential reward from the asteroid.
 */
typedef struct AsteroidReward {
   CommodityRef material; /**< Material dropped. */
   int          quantity; /**< Maximum amount. */
   int          rarity;   /**< Rarity. */
} AsteroidReward;

typedef struct AsteroidGfx {
   glTexture *gfx;
   CollPoly  *polygon;
} AsteroidGfx;

/**
 * @brief Represents a type of asteroid.
 */
typedef struct AsteroidType {
   char           *name;        /**< Name of the asteroid type. */
   char           *scanned_msg; /**< Scanned message. */
   AsteroidGfx    *gfxs;
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
typedef struct AsteroidTypeGroup {
   char          *name;    /**< Name of the type group. */
   AsteroidType **types;   /**< Types of asteroids in the group. */
   double        *weights; /**< Weights of each element in the group. */
   double         wtotal;  /**< Sum of weights in the group. */
} AsteroidTypeGroup;

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

typedef struct AsteroidVecStorage {
   Asteroid a;
} AsteroidVecStorage;
