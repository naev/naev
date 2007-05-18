

#ifndef SHIP_H
#  define SHIP_H


#include "opengl.h"
#include "outfit.h"

/* target gfx dimensions */
#define SHIP_TARGET_W	128
#define SHIP_TARGET_H	96


enum ship_class { SHIP_CLASS_NULL,
	SHIP_CLASS_CIV_LIGHT,
	SHIP_CLASS_CIV_MEDIUM,
	SHIP_CLASS_CIV_HEAVY
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


int ships_load(void);
void ships_free(void);


Ship* get_ship( const char* name );


#endif /* SHIP_H */
