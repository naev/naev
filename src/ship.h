/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SHIP_H
#  define SHIP_H


#include "opengl.h"
#include "outfit.h"
#include "sound.h"


/* target gfx dimensions */
#define SHIP_TARGET_W	128
#define SHIP_TARGET_H	96


typedef enum ShipClass_ {
	SHIP_CLASS_NULL=0,
	SHIP_CLASS_CIV_LIGHT=1,
	SHIP_CLASS_CIV_MEDIUM=2,
	SHIP_CLASS_CIV_HEAVY=3
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

	unsigned int price; /* cost to buy */

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


/*
 * toolkit
 */
void ship_view( char* shipname );


#endif /* SHIP_H */
