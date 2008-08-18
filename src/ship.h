/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SHIP_H
#  define SHIP_H


#include "opengl.h"
#include "outfit.h"
#include "sound.h"


/* target gfx dimensions */
#define SHIP_TARGET_W   128 /**< Ship target graphic width. */
#define SHIP_TARGET_H   96 /**< Ship target graphic height. */


/**
 * @typedef ShipClass
 *
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
   SHIP_CLASS_LUXERY_YACHT, /**< Small expensive ship. */
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
 * @struct ShipOutfit
 *
 * @brief Little wrapper for outfits.
 */
typedef struct ShipOutfit_ {
   struct ShipOutfit_* next; /**< Linked list next. */
   Outfit* data; /**< Data itself. */
   int quantity; /**< Important difference. */
} ShipOutfit;


/**
 * @struct Ship
 *
 * @brief Represents a space ship.
 */
typedef struct Ship_ {

   char* name; /**< ship name */
   ShipClass class; /**< ship class */

   /* store stuff */
   int price; /**< cost to buy */
   int tech; /**< see space.h */
   char* fabricator; /**< company that makes it */
   char* description; /**< selling description */

   /* movement */
   double thrust; /**< Ship's thrust in "pixel/sec^2" */
   double turn; /**< Ship's turn in rad/s */
   double speed; /**< Ship's max speed in "pixel/sec" */

   /* graphics */
   glTexture *gfx_space; /**< Space sprite sheet. */
   glTexture *gfx_target; /**< Targetting window graphic. */

   /* GUI interface */
   char* gui; /**< Name of the GUI the ship uses by default. */

   /* sound */
   int sound; /**< Sound motor uses.  Unused atm. */

   /* characteristics */
   int crew; /**< Crew members. */
   int mass; /**< Mass in tons. */
   int fuel; /**< How many jumps by default. */

   /* health */
   double armour; /**< Maximum base armour in MJ. */
   double armour_regen; /**< Maximum armour regeneration in MJ/s. */
   double shield; /**< Maximum base shield in MJ. */
   double shield_regen; /**< Maximum shield regeneration in MJ/s. */
   double energy; /**< Maximum base energy in MJ. */
   double energy_regen; /**< Maximum energy regeneration in MJ/s. */

   /* capacity */
   int cap_cargo; /**< Cargo capacity if empty. */
   int cap_weapon;  /**< Weapon capacity with no outfits. */

   /* outfits */
   ShipOutfit* outfit; /**< Linked list of outfits. */

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
Ship** ship_getTech( int *n, const int* tech, const int techmax );
char* ship_class( Ship* s );
int ship_basePrice( Ship* s );


/*
 * toolkit
 */
void ship_view( char* shipname );


#endif /* SHIP_H */
