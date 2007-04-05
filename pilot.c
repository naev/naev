

#include "pilot.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "all.h"
#include "log.h"


/* stack of pilot ids to assure uniqueness */
static unsigned int pilot_id = 0;


/* stack of pilots */
static Pilot** pilot_stack;
static int pilots = 0;


extern void player_think( Pilot* pilot, const FP dt ); /* player.c */
extern void ai_think( Pilot* pilot ); /* ai.c */
static void pilot_update( Pilot* pilot, const FP dt );
static void pilot_render( Pilot* pilot );


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
	return NULL;
*/

/* Dichotomical search */
	int i,n;
	for (i=0, n=pilots/2; n > 0; n /= 2 )
		i += (pilot_stack[i+n]->id > id) ? 0 : n ;

	return (pilot_stack[i]->id == id) ? pilot_stack[i] : NULL;

}


/*
 * renders the pilot
 */
static void pilot_render( Pilot* pilot )
{
	int sprite;
	gl_texture* texture = pilot->ship->gfx_ship;

	/* get the sprite corresponding to the direction facing */
	sprite = (int)(pilot->solid->dir / (2.0*M_PI / (texture->sy*texture->sx)));
	
	gl_blitSprite( texture, &pilot->solid->pos, sprite % (int)texture->sx, sprite / (int)texture->sy );
}


/*
 * updates the Pilot
 */
static void pilot_update( Pilot* pilot, const FP dt )
{
	if (pilot->solid->dir > 2*M_PI) pilot->solid->dir -= 2*M_PI;
	if (pilot->solid->dir < 0.0) pilot->solid->dir += 2*M_PI;

	/* update the solid */
	pilot->solid->update( pilot->solid, dt );

	pilot_render( pilot );
}


/*
 * Initialize pilot
 *
 * @ ship : ship pilot will be flying
 * @ name : pilot's name, if NULL ship's name will be used
 * @ vel : initial velocity
 * @ pos : initial position
 * @ flags : used for tweaking the pilot
 */
void pilot_init( Pilot* pilot, Ship* ship, char* name,
		const Vector2d* vel, const Vector2d* pos, const int flags )
{
	pilot->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

	pilot->ship = ship;
	pilot->name = strdup((name==NULL)?ship->name:name);

	pilot->solid = solid_create(ship->mass, vel, pos);

	/* max shields/armor */
	pilot->armor = ship->armor;
	pilot->shield = ship->shield;
	pilot->energy = ship->energy;

	/* initially idle */
	pilot->action = NULL;

	if (flags & PILOT_PLAYER) {
		pilot->think = (void*)player_think; /* players don't need to think! :P */
		pilot->properties |= PILOT_PLAYER;
		player = pilot;
	}
	else
		pilot->think = ai_think;

	pilot->update = pilot_update;
}


/*
 * Creates a new pilot
 *
 * see pilot_init for parameters
 *
 * returns pilot's id
 */
unsigned int pilot_create( Ship* ship, char* name,
		const Vector2d* vel, const Vector2d* pos, const int flags )
{
	Pilot* dyn = MALLOC_ONE(Pilot);
	if (dyn == NULL) {
		WARN("Unable to allocate memory");
		return 0;;
	}
	pilot_init( dyn, ship, name, vel, pos, flags );

	/* add to the stack */
	pilot_stack = realloc( pilot_stack, ++pilots*sizeof(Pilot*) );
	pilot_stack[pilots-1] = dyn;

	return dyn->id;
}


/*
 * frees the pilot
 */
void pilots_free()
{
	int i;
	for (i=0; i < pilots; i++) {
		solid_free(pilot_stack[i]->solid);
		free(pilot_stack[i]->name);
		free(pilot_stack[i]);
	}
	free(pilot_stack);
}


/*
 * updates all the pilots
 */
void pilots_update( FP dt )
{
	int i;
	for ( i=pilots-1; i >= 0; i-- ) {
		if ( pilot_stack[i]->think != NULL )
			pilot_stack[i]->think(pilot_stack[i]);
		pilot_stack[i]->update( pilot_stack[i], dt );
	}
}

