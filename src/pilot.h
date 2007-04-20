


#ifndef PILOT_H
#  define PILOT_H


#include "physics.h"
#include "ship.h"
#include "ai.h"
/*#include "outfit.h"
#include "faction.h"*/


/* creation flags */
#define PILOT_PLAYER		1 /* pilot is a player */



/*
 * primary pilot structure
 */
typedef struct Pilot {

	unsigned int id; /* pilot's id, used for many functions */
	char* name; /* pilot's name (if unique) */

	/* object caracteristics */
	Ship* ship; /* ship pilot is flying */
	Solid* solid; /* associated solid (physics) */
	/* Outfit* outfit; */

	/* current health */
	double armor, shield, energy;

	/* associated functions */
	void (*update)(struct Pilot*, const double); /* updates the pilot */

	unsigned int properties; /* used for AI and others */

	/* AI */
	void (*think)(struct Pilot*); /* AI thinking for the pilot */
	Task* task; /* current action */
} Pilot;


extern Pilot* player; /* the player */
Pilot* get_pilot( unsigned int id );


/*
 * creation
 */
void pilot_init( Pilot* dest, Ship* ship, char* name,
		const Vector2d* vel, const Vector2d* pos, const int flags );
unsigned int pilot_create( Ship* ship, char* name,
		const Vector2d* vel, const Vector2d* pos, const int flags );

/*
 * cleanup
 */
void pilot_destroy(Pilot* p);
void pilots_free (void);


/*
 * update
 */
void pilots_update( double dt );


#endif /* PILOT_H */
