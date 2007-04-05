


#ifndef PILOT_H
#  define PILOT_H


#include "all.h"
#include "physics.h"
#include "ship.h"
/*#include "outfit.h"
#include "faction.h"*/


/* creation flags */
#define PILOT_PLAYER		1 /* pilot is a player */


/*
 * AI
 *
 * AI is based on an action list which contains the current action (FIFO)
 * actions will run the appropriate Lua code
 */
typedef enum { ACT_ATTACK, ACT_TRAVEL, ACT_BRAKE } action_type;

/* actions */
struct Action {
	struct Action *next;
	action_type type;
	void* target;
};
typedef struct Action Action;


/*
 * primary pilot structure
 */
struct Pilot {

	unsigned int id; /* pilot's id, used for many functions */
	char* name; /* pilot's name (if unique) */

	/* object caracteristics */
	Ship* ship; /* ship pilot is flying */
	Solid* solid; /* associated solid (physics) */
	/* Outfit* outfit; */

	/* current health */
	FP armor, shield, energy;

	/* associated functions */
	void (*update)(struct Pilot*, const FP); /* updates the pilot */

	unsigned int properties; /* used for AI and others */

	/* AI */
	void (*think)(struct Pilot*); /* AI thinking for the pilot */
	Action* action; /* current action */
};
typedef struct Pilot Pilot;


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
void pilots_free(void);


/*
 * update
 */
void pilots_update( FP dt );


#endif /* PILOT_H */
