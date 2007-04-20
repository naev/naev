

#ifndef OUTFIT_H
#  define OUTFIT_H


#include "opengl.h"


/*
 * all the different outfit types
 */
typedef enum { OUTFIT_TYPE_NULL=0,
		OUTFIT_TYPE_BOLT,
		OUTFIT_TYPE_BEAM,
		OUTFIT_TYPE_MISSILE_DUMB,
		OUTFIT_TYPE_MISSILE_SEEK,
		OUTFIT_TYPE_MISSILE_SEEK_SMART,
		OUTFIT_TYPE_MISSILE_SWARM,
		OUTFIT_TYPE_MISSILE_SWARM_SMART
} OutfitType;

/*
 * an outfit, depends radically on the type
 */
typedef struct {
	char* name;

	int max;
	int tech;
	int mass;

	gl_texture gfx_store;

	OutfitType type;
	union {
		struct { /* OUTFIT_TYPE_LASER */
			double speed;
			double accuracy;
			double damage_armor, damage_shield;

			gl_texture* gfx_space;
		};
	};
} Outfit;


Outfit* outfit_get( const char* name );
int outfit_isweapon( const Outfit* o );
const char* outfit_getType( const Outfit* o );
/*
 * loading/freeing outfit stack
 */
int outfit_load (void);
void outfit_free (void);


#endif /* OUTFIT_H */
