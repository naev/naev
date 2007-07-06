

#include "weapon.h"

#include <math.h>
#include <malloc.h>
#include <string.h>

#include "main.h"
#include "log.h"
#include "rng.h"
#include "pilot.h"


/*
 * pilot stuff
 */
extern Pilot** pilot_stack;
extern int pilots;

typedef struct Weapon {
	Solid* solid; /* actually has its own solid :) */

	unsigned int parent; /* pilot that shot it */
	const Outfit* outfit; /* related outfit that fired it or whatnot */

	unsigned int timer; /* mainly used to see when the weapon was fired */

	/* position update and render */
	void (*update)(struct Weapon*, const double, WeaponLayer);
	void (*think)(struct Weapon*); /* for the smart missiles */

} Weapon;


/* behind pilots layer */
static Weapon** wbackLayer = NULL; /* behind pilots */
static int nwbackLayer = 0; /* number of elements */
static int mwbacklayer = 0; /* alloced memory size */
/* behind player layer */
static Weapon** wfrontLayer = NULL; /* infront of pilots, behind player */
static int nwfrontLayer = 0; /* number of elements */
static int mwfrontLayer = 0; /* alloced memory size */


/*
 * Prototypes
 */
static Weapon* weapon_create( const Outfit* outfit, const double dir,
		const Vector2d* pos, const Vector2d* vel, unsigned int parent );
static void weapon_render( const Weapon* w );
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer );
static void weapon_destroy( Weapon* w, WeaponLayer layer );
static void weapon_free( Weapon* w );


/*
 * draws the minimap weapons (used in player.c)
 */
void weapon_minimap( double res, double w, double h )
{
	int i;
	double x, y;
	for (i=0; i<nwbackLayer; i++) {
		x = (wbackLayer[i]->solid->pos.x - player->solid->pos.x) / res;
		y = (wbackLayer[i]->solid->pos.y - player->solid->pos.y) / res;
		if (ABS(x) < w/2. && ABS(y) < h/2.)
			glVertex2d( x, y );
	}
	for (i=0; i<nwfrontLayer; i++) {
		x = (wfrontLayer[i]->solid->pos.x - player->solid->pos.x) / res;
		y = (wfrontLayer[i]->solid->pos.y - player->solid->pos.y) / res;
		if (ABS(x) < w/2. && ABS(y) < h/2.)
			glVertex2d( x, y );
	}
}


/*
 * updates all the weapons in the layer
 */
void weapons_update( const double dt, WeaponLayer layer )
{
	Weapon** wlayer;
	int* nlayer;

	switch (layer) {
		case WEAPON_LAYER_BG:
			wlayer = wbackLayer;
			nlayer = &nwbackLayer;
			break;
		case WEAPON_LAYER_FG:
			wlayer = wfrontLayer;
			nlayer = &nwfrontLayer;
			break;
	}

	int i;
	Weapon* w;
	for (i=0; i<(*nlayer); i++) {
		w = wlayer[i];
		switch (wlayer[i]->outfit->type) {
			case OUTFIT_TYPE_BOLT:
				if (SDL_GetTicks() >
						(wlayer[i]->timer + 1000*(unsigned int)
						wlayer[i]->outfit->range/wlayer[i]->outfit->speed)) {
					weapon_destroy(wlayer[i],layer);
					continue;
				}
				break;

			default:
				break;
		}
		weapon_update(wlayer[i],dt,layer);

		/* if the weapon has been deleted we have to hold back one */
		if (w != wlayer[i]) i--;
	}
}


/*
 * renders the weapon
 */
static void weapon_render( const Weapon* w )
{
	int sprite;
	gl_texture* t = w->outfit->gfx_space;

	/* get the sprite corresponding to the direction facing */
	sprite = (int)(w->solid->dir / (2.0*M_PI / (t->sy*t->sx)));

	gl_blitSprite( t, &w->solid->pos, sprite % (int)t->sx, sprite / (int)t->sy );
}


/*
 * updates the weapon
 */
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer )
{
	int i;
	for (i=0; i<pilots; i++)
		if ( (w->parent != pilot_stack[i]->id) && /* pilot didn't shoot it */
				!areAllies(pilot_get(w->parent)->faction,pilot_stack[i]->faction) &&
				(DIST(w->solid->pos,pilot_stack[i]->solid->pos) < (PILOT_SIZE_APROX *
					w->outfit->gfx_space->sw/2. + pilot_stack[i]->ship->gfx_space->sw/2.))) {
			pilot_hit(pilot_stack[i], w->outfit->damage_shield, w->outfit->damage_armor);
			weapon_destroy(w,layer);
			return;
		}

	if (w->think) (*w->think)(w);
	
	(*w->solid->update)(w->solid, dt);

	weapon_render(w);
}


/*
 * creates a new weapon
 */
static Weapon* weapon_create( const Outfit* outfit, const double dir,
		const Vector2d* pos, const Vector2d* vel, unsigned int parent )
{
	Vector2d v;
	double mass = 1; /* presume lasers have a mass of 1 */
	double rdir = dir; /* real direction (accuracy) */
	Weapon* w = MALLOC_ONE(Weapon);
	w->parent = parent; /* non-changeable */
	w->outfit = outfit; /* non-changeable */
	w->update = weapon_update;
	w->timer = SDL_GetTicks();
	w->think = NULL;

	switch (outfit->type) {
		case OUTFIT_TYPE_BOLT:
			rdir += RNG(-outfit->accuracy/2.,outfit->accuracy/2.)/180.*M_PI;
			if ((rdir > 2.*M_PI) || (rdir < 0.)) rdir = fmod(rdir, 2.*M_PI);
			vect_cset( &v, VX(*vel)+outfit->speed*cos(rdir),
					VANGLE(*vel)+outfit->speed*sin(rdir));
			w->solid = solid_create( mass, rdir, pos, &v );
			break;

		default:
			w->solid = solid_create( mass, dir, pos, vel );
			break;
	}

	return w;
}


/*
 * adds a new weapon
 */
void weapon_add( const Outfit* outfit, const double dir,
		const Vector2d* pos, const Vector2d* vel,
		unsigned int parent, WeaponLayer layer )
{
	if (!outfit_isWeapon(outfit)) {
		ERR("Trying to create a Weapon from a non-Weapon type Outfit");
		return;
	}

	Weapon* w = weapon_create( outfit, dir, pos, vel, parent );

	/* set the proper layer */
	Weapon** curLayer = NULL;
	int *mLayer = NULL;
	int *nLayer = NULL;
	switch (layer) {
		case WEAPON_LAYER_BG:
			curLayer = wbackLayer;
			nLayer = &nwbackLayer;
			mLayer = &mwbacklayer;
			break;
		case WEAPON_LAYER_FG:
			curLayer = wfrontLayer;
			nLayer = &nwfrontLayer;
			mLayer = &mwfrontLayer;
			break;

		default:
			ERR("Invalid WEAPON_LAYER specified");
			return;
	}

	if (*mLayer > *nLayer) /* more memory alloced then needed */
		curLayer[(*nLayer)++] = w;
	else { /* need to allocate more memory */
		switch (layer) {
			case WEAPON_LAYER_BG:
				curLayer = wbackLayer = realloc(curLayer, (++(*mLayer))*sizeof(Weapon*));
				break;
			case WEAPON_LAYER_FG:
				curLayer = wfrontLayer = realloc(curLayer, (++(*mLayer))*sizeof(Weapon*));
				break;
		}
		curLayer[(*nLayer)++] = w;
	}
}


/*
 * destroys the weapon
 */
static void weapon_destroy( Weapon* w, WeaponLayer layer )
{
	int i;
	Weapon** wlayer;
	int *nlayer;
	switch (layer) {
		case WEAPON_LAYER_BG:
			wlayer = wbackLayer;
			nlayer = &nwbackLayer;
			break;
		case WEAPON_LAYER_FG:
			wlayer = wfrontLayer;
			nlayer = &nwfrontLayer;
			break;
	}

	for (i=0; wlayer[i] != w; i++); /* get to the current position */
	weapon_free(wlayer[i]);
	wlayer[i] = NULL;
	(*nlayer)--;

	for ( ; i < (*nlayer); i++)
		wlayer[i] = wlayer[i+1];
}


/*
 * clears the weapon
 */
static void weapon_free( Weapon* w )
{
	solid_free(w->solid);
	free(w);
}

/*
 * clears all the weapons, does NOT free the layers
 */
void weapon_clear (void)
{
	int i;
	for (i=0; i < nwbackLayer; i++)
		weapon_free(wbackLayer[i]);
	nwbackLayer = 0;
	for (i=0; i < nwfrontLayer; i++)
		weapon_free(wfrontLayer[i]);                                          
	nwfrontLayer = 0;
}

void weapon_exit (void)
{
	weapon_clear();
	free(wbackLayer);
	free(wfrontLayer);
}


