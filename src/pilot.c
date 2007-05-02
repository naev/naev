

#include "pilot.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "main.h"
#include "log.h"
#include "weapon.h"


/* stack of pilot ids to assure uniqueness */
static unsigned int pilot_id = 0;


/* stack of pilots */
static Pilot** pilot_stack;
static int pilots = 0;
extern Pilot* player;


/*
 * prototyes
 */
/* external */
extern void ai_destroy( Pilot* p ); /* ai.c */
extern void player_think( Pilot* pilot ); /* player.c */
extern void ai_think( Pilot* pilot ); /* ai.c */
/* internal */
static void pilot_update( Pilot* pilot, const double dt );
void pilot_render( Pilot* pilot ); /* externed in player.c */
static void pilot_free( Pilot* p );


/*
 * pulls a pilot out of the pilot_stack based on id
 */
Pilot* get_pilot( unsigned int id )
{
/* Regular search */
/*	int i;
	for ( i=0; i < pilots; i++ )
		if (pilot_stack[i]->id == id)
			return pilot_stack[i];
	return NULL;*/

	if (id==0) return player;

/* Dichotomical search */
	int i,n;
	for (i=0, n=pilots/2; n > 0; n /= 2 )
		i += (pilot_stack[i+n]->id > id) ? 0 : n ;

	return (pilot_stack[i]->id == id) ? pilot_stack[i] : NULL ;
}


/*
 * makes the pilot shoot
 *
 * @param p the pilot which is shooting
 * @param secondary whether they are shooting secondary weapons or primary weapons
 */
void pilot_shoot( Pilot* p, int secondary )
{
	int i;
	if (!secondary) { /* primary weapons */

		if (!p->outfits) return; /* no outfits */

		for (i=0; p->outfits[i].outfit; i++) /* cycles through outfits to find weapons */
			if (outfit_isWeapon(p->outfits[i].outfit) || /* is a weapon or launche */
					outfit_isLauncher(p->outfits[i].outfit))
				/* ready to shoot again */
				if ((SDL_GetTicks()-p->outfits[i].timer) >
						(p->outfits[i].outfit->delay/p->outfits[i].quantity))

					/* different weapons, different behaviours */
					switch (p->outfits[i].outfit->type) {
						case OUTFIT_TYPE_BOLT:
							weapon_add( p->outfits[i].outfit, p->solid->dir,
									&p->solid->pos, &p->solid->vel,
									(p==player) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG );
							p->outfits[i].timer = SDL_GetTicks();
							break;

						default:
							break;
					}
	}
}


/*
 * renders the pilot
 */
void pilot_render( Pilot* p )
{
	int sprite;
	gl_texture* t = p->ship->gfx_ship;

	/* get the sprite corresponding to the direction facing */
	sprite = (int)(p->solid->dir / (2.0*M_PI / (t->sy*t->sx)));

	gl_blitSprite( t, &p->solid->pos, sprite % (int)t->sx, sprite / (int)t->sy );
}


/*
 * updates the Pilot
 */
static void pilot_update( Pilot* pilot, const double dt )
{
	if (pilot->solid->dir > 2*M_PI) pilot->solid->dir -= 2*M_PI;
	if (pilot->solid->dir < 0.0) pilot->solid->dir += 2*M_PI;

	/* update the solid */
	pilot->solid->update( pilot->solid, dt );

	if (VMOD(pilot->solid->vel) > pilot->ship->speed) /* shouldn't go faster */
		vect_pset( &pilot->solid->vel, pilot->ship->speed, VANGLE(pilot->solid->vel) );

	pilot_render( pilot );
}


/*
 * Initialize pilot
 *
 * @ ship : ship pilot will be flying
 * @ name : pilot's name, if NULL ship's name will be used
 * @ dir : initial direction to face (radians)
 * @ vel : initial velocity
 * @ pos : initial position
 * @ flags : used for tweaking the pilot
 */
void pilot_init( Pilot* pilot, Ship* ship, char* name, const double dir,
		const Vector2d* pos, const Vector2d* vel, const int flags )
{
	if (flags & PILOT_PLAYER) /* player is ID 0 */
		pilot->id = 0;
	else
		pilot->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

	pilot->ship = ship;
	pilot->name = strdup( (name==NULL) ? ship->name : name );

	pilot->solid = solid_create(ship->mass, dir, pos, vel);

	/* max shields/armor */
	pilot->armor = ship->armor;
	pilot->shield = ship->shield;
	pilot->energy = ship->energy;

	/* initially idle */
	pilot->task = NULL;

	/* outfits */
	pilot->outfits = NULL;
	ShipOutfit* so;
	if (ship->outfit) {
		int noutfits = 0;
		for (so=ship->outfit; so; so=so->next) {
			pilot->outfits = realloc(pilot->outfits, (noutfits+1)*sizeof(PilotOutfit));
			pilot->outfits[noutfits].outfit = so->data;
			pilot->outfits[noutfits].quantity = so->quantity;
			pilot->outfits[noutfits].timer = 0;
			noutfits++;
		}
		/* sentinal */
		pilot->outfits = realloc(pilot->outfits, (noutfits+1)*sizeof(PilotOutfit));
		pilot->outfits[noutfits].outfit = NULL;
		pilot->outfits[noutfits].quantity = 0;
		pilot->outfits[noutfits].timer = 0;
	}


	if (flags & PILOT_PLAYER) {
		pilot->think = player_think; /* players don't need to think! :P */
		pilot->render = NULL;
		pilot->properties |= PILOT_PLAYER;
		player = pilot;
	}
	else {
		pilot->think = ai_think;
		pilot->render = pilot_render;
	}

	pilot->update = pilot_update;
}


/*
 * Creates a new pilot
 *
 * see pilot_init for parameters
 *
 * returns pilot's id
 */
unsigned int pilot_create( Ship* ship, char* name, const double dir,
		const Vector2d* pos, const Vector2d* vel, const int flags )
{
	Pilot* dyn = MALLOC_ONE(Pilot);
	if (dyn == NULL) {
		WARN("Unable to allocate memory");
		return 0;;
	}
	pilot_init( dyn, ship, name, dir, pos, vel, flags );

	if (flags & PILOT_PLAYER) { /* player */
		if (!pilot_stack) {
			pilot_stack = MALLOC_ONE(Pilot*);
			pilots = 1;
		}
		pilot_stack[0] = dyn;
	}
	else { /* add to the stack */
		pilot_stack = realloc( pilot_stack, ++pilots*sizeof(Pilot*) );
		pilot_stack[pilots-1] = dyn;
	}

	return dyn->id;
}


/*
 * frees and cleans up a pilot
 */
static void pilot_free( Pilot* p )
{
	solid_free(p->solid);
	free(p->outfits);
	free(p->name);
	ai_destroy(p);
	free(p);
}


/*
 * destroys pilot from stack
 */
void pilot_destroy(Pilot* p)
{
	int i;

	for (i=0; i < pilots; i++)
		if (pilot_stack[i]==p)
			break;

	while (i < pilots) {
		pilot_stack[i] = pilot_stack[i+1];
		i++;
	}

	pilot_free(p);
}


/*
 * frees the pilots
 */
void pilots_free (void)
{
	int i;
	for (i=0; i < pilots; i++)
		pilot_free(pilot_stack[i]);
	free(pilot_stack);
}


/*
 * updates all the pilots
 */
void pilots_update( double dt )
{
	int i;
	for ( i=0; i < pilots; i++ ) {
		if ( pilot_stack[i]->think)
			pilot_stack[i]->think(pilot_stack[i]);
		if (pilot_stack[i]->update)
			pilot_stack[i]->update( pilot_stack[i], dt );
		if (pilot_stack[i]->render)
			pilot_stack[i]->render(pilot_stack[i]);
	}
}

