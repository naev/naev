/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "collision.h"
#include "commodity.h"
#include "gltf.h"
#include "opengl.h"
#include "outfit.h"
#include "spfx.h"
#include "vec3.h"

/* Ship Flags. */
#define SHIP_NOPLAYER                                                          \
   ( 1 << 0 ) /**< Player is not allowed to fly the ship.                      \
               */
#define SHIP_NOESCORT                                                          \
   ( 1 << 1 ) /**< Player is not allowed to set the ship as an escort. */
#define SHIP_UNIQUE                                                            \
   ( 1 << 2 ) /**< Ship is unique and player can only have one. */
#define SHIP_NEEDSGFX ( 1 << 3 ) /**< Ship needs to load graphics. */
#define SHIP_3DTRAILS ( 1 << 4 ) /**< Ship is using 3D trails. */
#define SHIP_3DMOUNTS ( 1 << 5 ) /**< Ship is using 3D mounts. */
#define ship_isFlag( s, f ) ( ( s )->flags & ( f ) )   /**< Checks ship flag. */
#define ship_setFlag( s, f ) ( ( s )->flags |= ( f ) ) /**< Sets ship flag. */
#define ship_rmFlag( s, f )                                                    \
   ( ( s )->flags &= ~( f ) ) /**< Removes ship flag. */

/**
 * @brief Contains the different types of ships.
 *
 * See docs/ships/classification for more details.
 *
 * @sa ship_classFromString
 * @sa ship_class
 */
typedef enum ShipClass_ {
   SHIP_CLASS_NULL, /**< Invalid ship class. */
   /* Civilian. */
   SHIP_CLASS_YACHT,              /**< Civilian ultra-small ship. */
   SHIP_CLASS_COURIER,            /**< Civilian small ship. */
   SHIP_CLASS_FREIGHTER,          /**< Civilian medium ship. */
   SHIP_CLASS_ARMOURED_TRANSPORT, /**< Civilian edium, somewhat combat-oriented
                                     ship. */
   SHIP_CLASS_BULK_FREIGHTER,     /**< Civilian large ship. */
   /* Military. */
   SHIP_CLASS_SCOUT,       /**< Small scouter. */
   SHIP_CLASS_INTERCEPTOR, /**< Ultra small attack ship. */
   SHIP_CLASS_FIGHTER,     /**< Small attack ship. */
   SHIP_CLASS_BOMBER,      /**< Small attack ship with many missiles. */
   SHIP_CLASS_CORVETTE,    /**< Very agile medium ship. */
   SHIP_CLASS_DESTROYER,   /**< Not so agile medium ship. */
   SHIP_CLASS_CRUISER,     /**< Large ship. */
   SHIP_CLASS_BATTLESHIP,  /**< Larger ship. */
   SHIP_CLASS_CARRIER,     /**< Large ship with fighter bays. */
   /** @todo hybrid ship classification. */
   SHIP_CLASS_TOTAL, /**< Sentinal for total amount of ship classes. */
} ShipClass;

/**
 * @brief Represents a ship weapon mount point.
 */
typedef struct ShipMount_ {
   vec3 pos; /**< Mount position. */
} ShipMount;

/**
 * @brief Ship outfit slot.
 */
typedef struct ShipOutfitSlot_ {
   OutfitSlot slot;      /**< Outfit slot type. */
   char      *name;      /**< Name of the slot if applicable. */
   int        exclusive; /**< Outfits must match property to fit. */
   int required; /**< Outfit slot must be equipped for the ship to work. */
   int locked;   /**< Outfit slot is locked. */
   int visible;  /**< Outfit slot is always visible, even if locked. */
   const Outfit *data;  /**< Outfit by default if applicable. */
   ShipMount     mount; /**< Mountpoint, only used for weapon slots. */
} ShipOutfitSlot;

#define SHIP_TRAIL_ALWAYS_UNDER                                                \
   ( 1 << 0 ) /**< Should this trail be always drawn under the ship? */

/**
 * @brief Ship trail emitter.
 */
typedef struct ShipTrailEmitter_ {
   vec3             pos;        /**< Position. */
   unsigned int     flags;      /**< Flags to use. */
   const TrailSpec *trail_spec; /**< Trail type to emit. */
} ShipTrailEmitter;

/**
 * @brief Represents a space ship.
 */
typedef struct Ship {
   void *rawdata;   /**< Raw data trick. */
   char *inherits;  /**< Ship from which it is inheriting values. */
   char *name;      /**< Ship name. */
   char *base_type; /**< Ship's base type, basically used for figuring out what
                       ships are related. */
   char *base_path; /**< Ship's base type path, used for finding graphics and
                       such. */
   ShipClass class; /**< Ship class. */
   char *
      class_display; /**< Custom ship class, overrides class when displaying. */
   int faction;      /**< Dominant faction of the ship. */
   int
      points; /**< Number of points the ship costs (used for presence et al.) */
   int rarity; /**< Rarity. */
   int flags;  /**< Ship flags. */

   /* store stuff */
   credits_t price;   /**< Cost to buy. */
   char     *license; /**< License needed to buy it. */
   char *cond; /**< Conditional string to see if it can be bought. Must return a
                  boolean. */
   char *condstr;     /**< Human readable conditional. */
   char *fabricator;  /**< Company or organization that fabricates it. */
   char *description; /**< Shipyard description. */
   char *desc_extra;  /**< Extra description. */

   /* movement */
   double accel; /**< Ship's acceleration in "pixel/sec^2" */
   double turn;  /**< Ship's turn in rad/s */
   double speed; /**< Ship's max speed in "pixel/sec" */

   /* characteristics */
   int    crew;             /**< Crew members. */
   double mass;             /**< Mass ship has. */
   double cpu;              /**< Amount of CPU the ship has. */
   int    fuel;             /**< How much fuel by default. */
   int    fuel_consumption; /**< Fuel consumption by engine. */
   double cap_cargo;        /**< Cargo capacity (in volume). */
   double dt_default;       /**< Default/minimum time delta. */

   /* health */
   double armour;       /**< Maximum base armour in MJ. */
   double armour_regen; /**< Maximum armour regeneration in MJ/s. */
   double shield;       /**< Maximum base shield in MJ. */
   double shield_regen; /**< Maximum shield regeneration in MJ/s. */
   double energy;       /**< Maximum base energy in MJ. */
   double energy_regen; /**< Maximum energy regeneration in MJ/s. */
   double dmg_absorb; /**< Damage absorption in per one [0:1] with 1 being 100%
                         absorption. */

   /* Graphics */
   double      size;         /**< Size of the ship. */
   char       *gfx_path;     /**< Path to load GFX from (lazy loading). */
   char       *polygon_path; /**< Path to load polygon. */
   int         noengine;     /**< Don't try to load engine graphics. */
   GltfObject *gfx_3d;       /**< 3d model of the ship */
   glTexture  *gfx_space;    /**< Space sprite sheet. */
   glTexture  *gfx_engine;   /**< Space engine glow sprite sheet. */
   glTexture  *_gfx_store;   /**< Store graphic. */
   char       *gfx_comm;     /**< Name of graphic for communication. */
   glTexture **gfx_overlays; /**< Array (array.h): Store overlay graphics. */
   ShipTrailEmitter *trail_emitters; /**< Trail emitters. */
   int               sx; /* TODO remove this and sy when possible. */
   int               sy;

   /* Collision polygon */
   CollPoly polygon; /**< Array (array.h): Collision polygons. */

   /* Sound */
   int    sound;        /**< Sound engine uses. */
   double engine_pitch; /**< Sets the base pitch of the engine. */

   /* Outfits */
   ShipOutfitSlot
      *outfit_structure; /**< Array (array.h): Outfit structure slots. */
   ShipOutfitSlot
      *outfit_utility;            /**< Array (array.h): Outfit utility slots. */
   ShipOutfitSlot *outfit_weapon; /**< Array (array.h): Outfit weapons slots. */
   Outfit const  **outfit_intrinsic; /**< Array (array.h): Intrinsic outfits to
                                        start out with. */

   /* Statistics. */
   char         *desc_stats;  /**< Ship statistics information. */
   ShipStatList *stats;       /**< Ship statistics properties. */
   ShipStats     stats_array; /**< Laid out stats for referencing purposes. */

   /* Tags. */
   char **tags; /**< Ship tags. */

   /* Lua function references. Set to LUA_NOREF if not used. */
   char *lua_file; /**< Lua File. */
   nlua_env *
      lua_env; /**< Lua environment. Shared for each outfit to allow globals. */
   double lua_dt;             /**< Update rate for Lua update script. */
   int    lua_descextra;      /**< Run when obtaining description. */
   int    lua_init;           /**< Run when pilot enters a system. */
   int    lua_cleanup;        /**< Run when the pilot is erased. */
   int    lua_update;         /**< Run periodically. */
   int    lua_explode_init;   /**< Run when starting explosion. */
   int    lua_explode_update; /**< Run when exploding. */
   int    lua_onshootany;     /**< Run when any weapon is shot. */
   int    lua_cooldown;       /**< Run when pilot completely cools down. */
} Ship;

/*
 * Load/quit
 */
int  ships_load( void );
void ships_free( void );

/*
 * Getters.
 */
const Ship *ship_get( const char *name );
const Ship *ship_getW( const char *name );
const char *ship_existsCase( const char *name );
Ship       *ship_getAll( void );
const char *ship_class( const Ship *s );
const char *ship_classDisplay( const Ship *s );
const char *ship_classToString( ShipClass class );
ShipClass   ship_classFromString( const char *str );
credits_t   ship_basePrice( const Ship *s );
credits_t   ship_buyPrice( const Ship *s );
int         ship_size( const Ship *s );

/*
 * Rendering.
 */
void ship_renderFramebuffer( const Ship *s, GLuint fbo, double fw, double fh,
                             double dir, double engine_glow, double tilt,
                             double r, int sx, int sy, const glColour *c,
                             const Lighting *L );
USE_RESULT glTexture *ship_gfxComm( const Ship *s, int size, double tilt,
                                    double dir, const Lighting *Lscene );
void ship_renderGfxStore( GLuint fbo, const Ship *s, int size, double dir,
                          double updown, double glow );
USE_RESULT glTexture *ship_gfxStore( const Ship *s, int size, double dir,
                                     double updown, double glow );
int                   ship_gfxAnimated( const Ship *s );

/*
 * Misc.
 */
void   ships_resize( void );
int    ship_gfxLoaded( const Ship *s );
int    ship_gfxLoadNeeded( void );
int    ship_gfxLoad( Ship *temp );
int    ship_compareTech( const void *arg1, const void *arg2 );
double ship_maxSize( void );
