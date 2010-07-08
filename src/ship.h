/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SHIP_H
#  define SHIP_H


#include "opengl.h"
#include "outfit.h"
#include "sound.h"
#include "nxml.h"


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
   SHIP_CLASS_NULL, /**< Invalid ship class. */
   /* Civilian. */
   SHIP_CLASS_YACHT, /**< Small cheap ship. */
   SHIP_CLASS_LUXURY_YACHT, /**< Small expensive ship. */
   SHIP_CLASS_CRUISE_SHIP, /**< Medium ship. */
   /* Merchant. */
   SHIP_CLASS_COURIER, /**< Small ship. */
   SHIP_CLASS_FREIGHTER, /**< Medium ship. */
   SHIP_CLASS_BULK_CARRIER, /**< Large ship. */
   /* Military. */
   SHIP_CLASS_SCOUT, /**< Small scouter. */
   SHIP_CLASS_FIGHTER, /**< Small attack ship. */
   SHIP_CLASS_BOMBER, /**< Small attack ship with many missiles. */
   SHIP_CLASS_CORVETTE, /**< Very agile medium ship. */
   SHIP_CLASS_DESTROYER, /**< Not so agile medium ship. */
   SHIP_CLASS_CRUISER, /**< Large ship. */
   SHIP_CLASS_CARRIER, /**< Large ship with fighter bays. */
   /* Robotic */
   SHIP_CLASS_DRONE, /**< Unmanned small robotic ship. */
   SHIP_CLASS_HEAVY_DRONE, /**< Unmanned medium robotic ship. */
   SHIP_CLASS_MOTHERSHIP /**< Unmanned large robotic carrier. */
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
   OutfitSlotType slot; /**< Type of slot. */
   Outfit *data; /**< Outfit by default if applicable. */
   ShipMount mount; /**< Mountpoint. */
} ShipOutfitSlot;


/**
 * @brief Represents a space ship.
 */
typedef struct Ship_ {
   char* name; /**< Ship name */
   char* base_type; /**< Ship's base type, basically used for figuring out what ships are related. */
   ShipClass class; /**< Ship class */

   /* store stuff */
   unsigned int price; /**< Cost to buy */
   char* license; /**< License needed to buy it. */
   char* fabricator; /**< company that makes it */
   char* description; /**< selling description */

   /* movement */
   double thrust; /**< Ship's thrust in "pixel/sec^2" */
   double turn; /**< Ship's turn in rad/s */
   double speed; /**< Ship's max speed in "pixel/sec" */

   /* characteristics */
   int crew; /**< Crew members. */
   double mass; /**< Mass ship has. */
   double cpu; /**< Amount of CPU the ship has. */
   int fuel; /**< How many jumps by default. */
   double cap_cargo; /**< Cargo capacity (in volume). */

   /* health */
   double armour; /**< Maximum base armour in MJ. */
   double armour_regen; /**< Maximum armour regeneration in MJ/s. */
   double shield; /**< Maximum base shield in MJ. */
   double shield_regen; /**< Maximum shield regeneration in MJ/s. */
   double energy; /**< Maximum base energy in MJ. */
   double energy_regen; /**< Maximum energy regeneration in MJ/s. */

   /* graphics */
   glTexture *gfx_space; /**< Space sprite sheet. */
   glTexture *gfx_engine; /**< Space engine glow sprite sheet. */
   glTexture *gfx_target; /**< Targetting window graphic. */
   char* gfx_comm; /**< Name of graphic for communication. */

   /* GUI interface */
   char* gui; /**< Name of the GUI the ship uses by default. */

   /* sound */
   int sound; /**< Sound motor uses. */

   /* outfits */
   int outfit_nlow; /**< Number of low energy outfit slots. */
   ShipOutfitSlot *outfit_low; /**< Outfit low energy slots. */
   int outfit_nmedium; /**< Number of medium energy outfit slots. */
   ShipOutfitSlot *outfit_medium; /**< Outfit medium energy slots. */
   int outfit_nhigh; /**< Number of high energy outfit slots. */
   ShipOutfitSlot *outfit_high; /**< Outfit high energy slots. */

   /* mounts */
   double mangle; /**< Mount angle to simplify mount calculations. */

   /* Statistics. */
   char *desc_stats; /**< Ship statistics information. */
   ShipStats stats; /**< Ship statistics properties. */
} Ship;


/*
 * load/quit
 */
int ships_load (void);
void ships_free (void);

/*
 * stats
 */
int ship_statsParse( ShipStats *s, xmlNodePtr parent );
int ship_statsDesc( ShipStats *s, char *buf, int len, int newline, int pilot );

/*
 * get
 */
Ship* ship_get( const char* name );
Ship* ship_getW( const char* name );
Ship* ship_getAll( int *n );
Ship** ship_getTech( int *n, const int* tech, const int techmax );
char* ship_class( Ship* s );
ShipClass ship_classFromString( char* str );
int ship_basePrice( Ship* s );
glTexture* ship_loadCommGFX( Ship* s );


/*
 * toolkit
 */
void ship_view( unsigned int unused, char* shipname );


/*
 * misc.
 */
int ship_compareTech( const void *arg1, const void *arg2 );


#endif /* SHIP_H */
