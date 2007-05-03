


#ifndef PILOT_H
#  define PILOT_H


#include "physics.h"
#include "ship.h"
#include "ai.h"
#include "outfit.h"
/*#include "faction.h"*/


/* creation flags */
#define PILOT_PLAYER		1 /* pilot is a player */


typedef struct {
	Outfit* outfit; /* associated outfit */
	unsigned int quantity; /* number of outfits of this type pilot has */

	unsigned int timer; /* used to store when it was last used */
} PilotOutfit;


/*
 * primary pilot structure
 */
typedef struct Pilot {

	unsigned int id; /* pilot's id, used for many functions */
	char* name; /* pilot's name (if unique) */

	/* object caracteristics */
	Ship* ship; /* ship pilot is flying */
	Solid* solid; /* associated solid (physics) */

	/* current health */
	double armor, shield, energy;

	/* associated functions */
	void (*think)(struct Pilot*); /* AI thinking for the pilot */
	void (*update)(struct Pilot*, const double); /* updates the pilot */
	void (*render)(struct Pilot*); /* for rendering the pilot */

	/* outfit management */
	PilotOutfit* outfits;

	unsigned int properties; /* used for AI and others */

	/* AI */
	Task* task; /* current action */
} Pilot;



/*
 * getting pilot stuff
 */
extern Pilot* player; /* the player */
Pilot* get_pilot( unsigned int id );


/*
 * misc
 */
void pilot_shoot( Pilot* p, int secondary );
void pilot_hit( Pilot* p, double damage_shield, double damage_armor );


/*
 * creation
 */
void pilot_init( Pilot* dest, Ship* ship, char* name, const double dir,
		const Vector2d* pos, const Vector2d* vel, const int flags );
unsigned int pilot_create( Ship* ship, char* name, const double dir,
		const Vector2d* pos, const Vector2d* vel, const int flags );

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
