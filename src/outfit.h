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
typedef enum OutfitType_ {
	OUTFIT_TYPE_NULL=0,
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
	OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO=12,
	OUTFIT_TYPE_TURRET_BOLT=13,
	OUTFIT_TYPE_TURRET_BEAM=14
} OutfitType;

/*
 * an outfit, depends radically on the type
 */
typedef struct Outfit_ {
	char* name;

	/* general specs */
	int max;
	int tech;
	int mass;

	/* store stuff */
	unsigned int price;
	char* description;

	glTexture gfx_store; /* store graphic */

	int properties; /* properties stored bitwise */

	/* Type dependent */
	OutfitType type;
	union {
		struct { /* bolt */
			unsigned int delay; /* delay between shots */
			double speed; /* how fast it goes (not applicable to beam) */
			double range; /* how far it goes */
			double accuracy; /* desviation accuracy */
			double damage_armour, damage_shield; /* damage */

			glTexture* gfx_space; /* graphic */
			ALuint sound; /* sound to play */
			int spfx; /* special effect on hit */
		} blt;
		struct { /* beam */
			double range; /* how far it goes */
			glColour colour; /* beam colour */
			double energy; /* energy it drains */
			double damage_armour, damage_shield; /* damage */
		} bem;
		struct { /* launcher */
			unsigned int delay; /* delay between shots */
			char *ammo; /* the ammo to use */
		} lau;
		struct { /* ammo */
			unsigned int duration; /* duration */
			double speed; /* maximum speed */
			double turn; /* turn velocity */
			double thrust; /* acceleration */
			double damage_armour, damage_shield; /* damage */

			glTexture* gfx_space; /* graphic */
			ALuint sound; /* sound to play */
			int spfx; /* special effect on hit */

			unsigned int lockon; /* time it takes to lock on the target */
		} amm;
	} u;
} Outfit;


/*
 * get
 */
Outfit* outfit_get( const char* name );
char** outfit_getAll( int *n );
/* outfit types */
int outfit_isWeapon( const Outfit* o );
int outfit_isLauncher( const Outfit* o );
int outfit_isAmmo( const Outfit* o );
int outfit_isTurret( const Outfit* o );
const char* outfit_getType( const Outfit* o );
const char* outfit_getTypeBroad( const Outfit* o );

/*
 * get data from outfit
 */
glTexture* outfit_gfx( const Outfit* o );
int outfit_spfx( const Outfit* o );
double outfit_dmgShield( const Outfit* o );
double outfit_dmgArmour( const Outfit* o );
int outfit_delay( const Outfit* o );

/*
 * loading/freeing outfit stack
 */
int outfit_load (void);
void outfit_free (void);


#endif /* OUTFIT_H */
