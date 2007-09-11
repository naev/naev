/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef OUTFIT_H
#  define OUTFIT_H


#include "opengl.h"
#include "sound.h"


/*
 * properties
 */
#define outfit_isProp(o,p)				((o)->properties & p)
/* property flags */
#define OUTFIT_PROP_WEAP_SECONDARY	(1<<0)


/*
 * all the different outfit types
 */
typedef enum { OUTFIT_TYPE_NULL=0,
		OUTFIT_TYPE_BOLT=1,
		OUTFIT_TYPE_BEAM=2,
		OUTFIT_TYPE_MISSILE_DUMB=3,
		OUTFIT_TYPE_MISSILE_DUMB_AMMO=4,
		OUTFIT_TYPE_MISSILE_SEEK=5,
		OUTFIT_TYPE_MISSILE_SEEK_AMMO=6,
		OUTFIT_TYPE_MISSILE_SEEK_SMART=7,
		OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO=8,
		OUTFIT_TYPE_MISSILE_SWARM=9,
		OUTFIT_TYPE_MISSILE_SWARM_AMMO=10,
		OUTFIT_TYPE_MISSILE_SWARM_SMART=11,
		OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO=12
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

	glTexture gfx_store; /* store graphic */

	int properties; /* properties stored bitwise */

	/* Type dependent */
	OutfitType type;
	union {
		struct { /* beam/bolt */
			unsigned int delay; /* delay between shots */
			double speed; /* how fast it goes (not applicable to beam) */
			double range; /* how far it goes */
			double accuracy; /* desviation accuracy */
			double damage_armour, damage_shield; /* damage */

			glTexture* gfx_space; /* graphic */
			ALuint sound; /* sound to play */
		};
		struct { /* launcher */
			unsigned int delay; /* delay between shots */
			char *ammo; /* the ammo to use */
		};
		struct { /* ammo */
			double speed; /* maximum speed */
			double turn; /* turn velocity */
			double thrust; /* acceleration */
			unsigned int duration; /* duration */
			double damage_armour, damage_shield; /* damage */

			glTexture* gfx_space; /* graphic */
			ALuint sound; /* sound to play */
		};
	};
} Outfit;


Outfit* outfit_get( const char* name );
/* outfit types */
int outfit_isWeapon( const Outfit* o );
int outfit_isLauncher( const Outfit* o );
int outfit_isAmmo( const Outfit* o );
const char* outfit_getType( const Outfit* o );
const char* outfit_getTypeBroad( const Outfit* o );

/*
 * loading/freeing outfit stack
 */
int outfit_load (void);
void outfit_free (void);


#endif /* OUTFIT_H */
