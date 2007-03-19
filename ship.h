

#ifndef SHIP_H
#  define SHIP_H


#include "all.h"
#include "opengl.h"


enum ship_class { SHIP_CLASS_NULL,
	SHIP_CLASS_CIVILIAN };
typedef enum ship_class ship_class;


typedef struct {

	char* name; /* ship name */
	ship_class class; /* ship class */

	/* movement */
	FP thrust, turn, speed;

	/* graphics */
	gl_texture *gfx_ship, *gfx_target;

	/* characteristics */
	int crew;
	int mass;

	/* health */
	FP armor, armor_regen;
	FP shield, shield_regen;
	FP energy, energy_regen;

	/* capacity */
	int cap_cargo, cap_weapon; 

} Ship;


int ships_load(void);
void ships_free(void);


Ship* get_ship( const char* name );


#endif /* SHIP_H */
