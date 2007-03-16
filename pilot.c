

#include "pilot.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>


/* stack of pilot ids to assure uniqueness */
static unsigned int pilot_id = 0;


static void pilot_update( Pilot* pilot, const FP dt );
static void pilot_render( Pilot* pilot );


/*
 * renders the pilot
 */
static void pilot_render( Pilot* pilot )
{
	int sprite;
	Vector2d pos;
	gl_texture* texture = pilot->ship->gfx_ship;

	pos.x = pilot->solid->pos.x;
	pos.y = pilot->solid->pos.y;

	/* get the sprite corresponding to the direction facing */
	sprite = (int)(pilot->solid->dir / (2.0*M_PI / (texture->sy*texture->sx)));
	
	gl_blitSprite( texture, &pos, sprite % (int)texture->sx, sprite / (int)texture->sy );
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
	pilot->id = pilot_id++; /* new unique pilot id based on pilot_id */

	pilot->ship = ship;
	if (name == NULL)
		pilot->name = ship->name;
	else
		pilot->name = name;

	pilot->solid = solid_create(ship->mass, vel, pos);

	pilot->armor = ship->armor;
	pilot->shield = ship->shield;
	pilot->energy = ship->energy;

	if (flags & PILOT_PLAYER)
		pilot->think = NULL; /* players don't need to think! :P */

	pilot->update = pilot_update;
}


/*
 * Creates a new pilot
 *
 * see pilot_init for parameters
 */
Pilot* pilot_create( Ship* ship, char* name,
		const Vector2d* vel, const Vector2d* pos, const int flags )
{
	Pilot* dyn = MALLOC_ONE(Pilot);
	assert(dyn != NULL);
	pilot_init( dyn, ship, name, vel, pos, flags );
	return dyn;
}


/*
 * frees the pilot
 */
void pilot_free( Pilot* pilot )
{
	solid_free( pilot->solid );
	free(pilot);
	pilot = NULL;
}
