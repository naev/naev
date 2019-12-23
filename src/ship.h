/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SHIP_H
#  define SHIP_H


#include "opengl.h"
#include "outfit.h"
#include "sound.h"
#include "nxml.h"
#include "economy.h"


/* target gfx dimensions */
#define SHIP_TARGET_W   128 /**< Ship target graphic width. */
#define SHIP_TARGET_H   96 /**< Ship target graphic height. */


/**
 * @brief Contains the different types of ships.
 *
 * See docs/ships/classification for more details.
 *
 * @sa ship_classFromString
 * @sa ship_class
 */
typedef enum ShipClass_ {
   SHIP_CLASS_NULL,        /**< Invalid ship class. */
   /* Civilian. */
   SHIP_CLASS_YACHT,       /**< Small cheap ship. */
   SHIP_CLASS_LUXURY_YACHT, /**< Small expensive ship. */
   SHIP_CLASS_CRUISE_SHIP, /**< Medium ship. */
   /* Merchant. */
   SHIP_CLASS_COURIER,     /**< Small ship. */
   SHIP_CLASS_ARMOURED_TRANSPORT, /**< Medium, somewhat combat-oriented ship. */
   SHIP_CLASS_FREIGHTER,   /**< Medium ship. */
   SHIP_CLASS_BULK_CARRIER, /**< Large ship. */
   /* Military. */
   SHIP_CLASS_SCOUT,       /**< Small scouter. */
   SHIP_CLASS_FIGHTER,     /**< Small attack ship. */
   SHIP_CLASS_BOMBER,      /**< Small attack ship with many missiles. */
   SHIP_CLASS_CORVETTE,    /**< Very agile medium ship. */
   SHIP_CLASS_DESTROYER,   /**< Not so agile medium ship. */
   SHIP_CLASS_CRUISER,     /**< Large ship. */
   SHIP_CLASS_CARRIER,     /**< Large ship with fighter bays. */
   /* Robotic */
   SHIP_CLASS_DRONE,       /**< Unmanned small robotic ship. */
   SHIP_CLASS_HEAVY_DRONE, /**< Unmanned medium robotic ship. */
   SHIP_CLASS_MOTHERSHIP   /**< Unmanned large robotic carrier. */
   /* Hybrid */
   /** @todo hybrid ship classification. */
} ShipClass;


/**
 * @brief Represents a ship weapon mount point.
 */
typedef struct ShipMount_ {
   double x; /**< X position of the mount point. */
   double y; /**< Y position of the mount point. */
   double h; /**< Mount point height (displacement). */
} ShipMount;


/**
 * @brief Ship outfit slot.
 */
typedef struct ShipOutfitSlot_ {
   OutfitSlot slot;  /**< Outfit slot type. */
   int exclusive;    /**< Outfits must match property to fit. */
   int required;     /**< Outfit slot must be equipped for the ship to work. */
   Outfit *data;     /**< Outfit by default if applicable. */
   ShipMount mount;  /**< Mountpoint, only used for weapon slots. */
} ShipOutfitSlot;


/**
 * @brief Represents a space ship.
 */
typedef struct Ship_ {
   char* name;       /**< Ship name */
   char* base_type;  /**< Ship's base type, basically used for figuring out what ships are related. */
   ShipClass class;  /**< Ship class */

   /* store stuff */
   credits_t price;  /**< Cost to buy */
   char* license;    /**< License needed to buy it. */
   char* fabricator; /**< company that makes it */
   char* description; /**< selling description */

   /* movement */
   double thrust;    /**< Ship's thrust in "pixel/sec^2" (not multiplied by mass) */
   double turn;      /**< Ship's turn in rad/s */
   double speed;     /**< Ship's max speed in "pixel/sec" */

   /* characteristics */
   int crew;                /**< Crew members. */
   double mass;             /**< Mass ship has. */
   double cpu;              /**< Amount of CPU the ship has. */
   int fuel;                /**< How much fuel by default. */
   int fuel_consumption; /**< Fuel consumption by engine. */
   double cap_cargo;        /**< Cargo capacity (in volume). */
   double dt_default;      /**< Default/minimum time delta. */

   /* health */
   double armour;    /**< Maximum base armour in MJ. */
   double armour_regen; /**< Maximum armour regeneration in MJ/s. */
   double shield;    /**< Maximum base shield in MJ. */
   double shield_regen; /**< Maximum shield regeneration in MJ/s. */
   double energy;    /**< Maximum base energy in MJ. */
   double energy_regen; /**< Maximum energy regeneration in MJ/s. */
   double dmg_absorb; /**< Damage absorption in per one [0:1] with 1 being 100% absorption. */

   /* graphics */
   glTexture *gfx_space; /**< Space sprite sheet. */
   glTexture *gfx_engine; /**< Space engine glow sprite sheet. */
   glTexture *gfx_target; /**< Targeting window graphic. */
   glTexture *gfx_store; /**< Store graphic. */
   char* gfx_comm;   /**< Name of graphic for communication. */

   /* GUI interface */
   char* gui;        /**< Name of the GUI the ship uses by default. */

   /* sound */
   int sound;        /**< Sound motor uses. */

   /* outfits */
   int outfit_nstructure; /**< Number of structure outfit slots. */
   ShipOutfitSlot *outfit_structure; /**< Outfit structure slots. */
   int outfit_nutility; /**< Number of utility outfit slots. */
   ShipOutfitSlot *outfit_utility; /**< Outfit utility slots. */
   int outfit_nweapon; /**< Number of weapon outfit slots. */
   ShipOutfitSlot *outfit_weapon; /**< Outfit weapons slots. */

   /* mounts */
   double mangle;    /**< Mount angle to simplify mount calculations. */

   /* Statistics. */
   char *desc_stats; /**< Ship statistics information. */
   ShipStatList *stats; /**< Ship statistics properties. */
   ShipStats stats_array; /**< Laid out stats for referencing purposes. */
} Ship;


/*
 * load/quit
 */
int ships_load (void);
void ships_free (void);

/*
 * get
 */
Ship* ship_get( const char* name );
Ship* ship_getW( const char* name );
const char *ship_existsCase( const char* name );
Ship* ship_getAll( int *n );
char* ship_class( Ship* s );
ShipClass ship_classFromString( char* str );
credits_t ship_basePrice( const Ship* s );
credits_t ship_buyPrice( const Ship* s );
glTexture* ship_loadCommGFX( Ship* s );


/*
 * misc.
 */
int ship_compareTech( const void *arg1, const void *arg2 );


#endif /* SHIP_H */
