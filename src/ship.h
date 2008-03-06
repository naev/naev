/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SHIP_H
#  define SHIP_H


#include "opengl.h"
#include "outfit.h"
#include "sound.h"


/* target gfx dimensions */
#define SHIP_TARGET_W   128
#define SHIP_TARGET_H   96


typedef enum ShipClass_ {
   SHIP_CLASS_NULL=0,
   /* CIVILIAN */
   SHIP_CLASS_CIV_LIGHT=1,
   SHIP_CLASS_CIV_MEDIUM=2,
   SHIP_CLASS_CIV_HEAVY=3,
   /* MILITARY */
   SHIP_CLASS_MIL_LIGHT=4,
   SHIP_CLASS_MIL_MEDIUM=5,
   SHIP_CLASS_MIL_HEAVY=6,
   /* ROBOTIC */
   SHIP_CLASS_ROB_LIGHT=7,
   SHIP_CLASS_ROB_MEDIUM=8,
   SHIP_CLASS_ROB_HEAVY=9,
   /* HYBRID */
   SHIP_CLASS_HYB_LIGHT=10,
   SHIP_CLASS_HYB_MEDIUM=11,
   SHIP_CLASS_HYB_HEAVY=12
} ShipClass;


/*
 * little wrapper for outfits
 */
typedef struct ShipOutfit_ {
   struct ShipOutfit_* next; /* linked list */
   Outfit* data; /* data itself */
   int quantity; /* important difference */
} ShipOutfit;


/*
 * ship class itself
 */
typedef struct Ship_ {

   char* name; /* ship name */
   ShipClass class; /* ship class */

   /* store stuff */
   int price; /* cost to buy */
   int tech; /* see space.h */
   char* fabricator; /* company that makes it */
   char* description; /* selling description */

   /* movement */
   double thrust, turn, speed;

   /* graphics */
   glTexture *gfx_space, *gfx_target;

   /* GUI interface */
   char* gui;

   /* sound */
   ALuint sound;

   /* characteristics */
   int crew;
   int mass;
   int fuel; /* how many jumps by default */

   /* health */
   double armour, armour_regen;
   double shield, shield_regen;
   double energy, energy_regen;

   /* capacity */
   int cap_cargo, cap_weapon; 

   /* outfits */
   ShipOutfit* outfit;

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
char** ship_getTech( int *n, const int* tech, const int techmax );
char* ship_class( Ship* s );


/*
 * toolkit
 */
void ship_view( char* shipname );


#endif /* SHIP_H */
