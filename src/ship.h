

#ifndef SHIP_H
#  define SHIP_H


#include "opengl.h"
#include "outfit.h"

/* target gfx dimensions */
#define SHIP_TARGET_W	128
#define SHIP_TARGET_H	96


enum ship_class { SHIP_CLASS_NULL=0,
	SHIP_CLASS_CIV_LIGHT=1,
	SHIP_CLASS_CIV_MEDIUM=2,
	SHIP_CLASS_CIV_HEAVY=3
};
typedef enum ship_class ship_class;


/*
 * little wrapper for outfits
 */
typedef struct ShipOutfit {
	struct ShipOutfit* next; /* linked list */
	Outfit* data; /* data itself */
	int quantity; /* important difference */
} ShipOutfit;


/*
 * ship class itself
 */
typedef struct {

	char* name; /* ship name */
	ship_class class; /* ship class */

	/* movement */
	double thrust, turn, speed;

	/* graphics */
	gl_texture *gfx_space, *gfx_target;

	/* GUI interface */
	char* gui;

	/* characteristics */
	int crew;
	int mass;

	/* health */
	double armor, armor_regen;
	double shield, shield_regen;
	double energy, energy_regen;

	/* capacity */
	int cap_cargo, cap_weapon; 

	/* outfits */
	ShipOutfit* outfit;

} Ship;


int ships_load (void);
void ships_free (void);


Ship* ship_get( const char* name );


#endif /* SHIP_H */
