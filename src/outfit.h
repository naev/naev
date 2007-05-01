

#ifndef OUTFIT_H
#  define OUTFIT_H


#include "opengl.h"


/*
 * properties
 */
#define OUTFIT_PROP_WEAP_PRIMARY		(1<<0)
#define OUTFIT_PROP_WEAP_SECONDARY	(1<<1)


/*
 * all the different outfit types
 */
typedef enum { OUTFIT_TYPE_NULL=0,
		OUTFIT_TYPE_BOLT,
		OUTFIT_TYPE_BEAM,
		OUTFIT_TYPE_MISSILE_DUMB,
		OUTFIT_TYPE_MISSILE_DUMB_AMMO,
		OUTFIT_TYPE_MISSILE_SEEK,
		OUTFIT_TYPE_MISSILE_SEEK_AMMO,
		OUTFIT_TYPE_MISSILE_SEEK_SMART,
		OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO,
		OUTFIT_TYPE_MISSILE_SWARM,
		OUTFIT_TYPE_MISSILE_SWARM_AMMO,
		OUTFIT_TYPE_MISSILE_SWARM_SMART,
		OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO
} OutfitType;

/*
 * an outfit, depends radically on the type
 */
typedef struct {
	char* name;

	/* general specs */
	int max;
	int tech;
	int mass;

	gl_texture gfx_store; /* store graphic */

	int properties; /* properties stored bitwise */

	/* Type dependent */
	OutfitType type;
	union {
		struct { /* beam/bolt */
			unsigned int delay; /* delay between shots */
			double speed; /* how fast it goes (not applicable to beam) */
			double range; /* how far it goes */
			double accuracy; /* desviation accuracy */
			double damage_armor, damage_shield; /* damage */

			gl_texture* gfx_space; /* graphic */
		};
		struct { /* launcher */
			unsigned int delay; /* delay between shots */
		};
		struct { /* ammo */
			double speed; /* maximum speed */
			double turn; /* turn velocity */
			double thrust; /* acceleration */
			double damage_armor, damage_shield; /* damage */

			gl_texture* gfx_space; /* graphic */
		};
	};
} Outfit;


Outfit* outfit_get( const char* name );
/* outfit types */
int outfit_isWeapon( const Outfit* o );
int outfit_isLauncher( const Outfit* o );
int outfit_isAmmo( const Outfit* o );
const char* outfit_getType( const Outfit* o );
/*
 * loading/freeing outfit stack
 */
int outfit_load (void);
void outfit_free (void);


#endif /* OUTFIT_H */
