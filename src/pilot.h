/*
 * See Licensing and Copyright notice in naev.h
 */




#ifndef PILOT_H
#  define PILOT_H


#include "physics.h"
#include "ship.h"
#include "ai.h"
#include "outfit.h"
#include "faction.h"
#include "sound.h"


#define PLAYER_ID			1


#define HYPERSPACE_FLY_DELAY		5000
#define HYPERSPACE_ENGINE_DELAY	3000


#define PILOT_SIZE_APROX		0.8	/* aproximation for pilot size */
#define PILOT_DISABLED_ARMOR	0.2	/* armour % that gets it disabled */


/* flags */
#define pilot_isFlag(p,f)  ((p)->flags & f)
#define pilot_setFlag(p,f) ((p)->flags |= f)
#define pilot_rmFlag(p,f)  ((p)->flags ^= f)
/* creation */
#define PILOT_PLAYER			(1<<0) /* pilot is a player */
/* dynamic */
#define PILOT_HOSTILE	   (1<<1) /* pilot is hostile to the player */
#define PILOT_COMBAT			(1<<2) /* pilot is engaged in combat */
#define PILOT_HYP_PREP		(1<<5) /* pilot is getting ready for hyperspace */
#define PILOT_HYP_BEGIN		(1<<6) /* pilot is starting engines */
#define PILOT_HYPERSPACE	(1<<7) /* pilot is in hyperspace */
#define PILOT_DISABLED		(1<<9) /* pilot is disabled */
#define PILOT_DELETE			(1<<10) /* pilot will get deleted asap */

/* makes life easier */
#define pilot_isPlayer(p)	((p)->flags & PILOT_PLAYER)
#define pilot_isDisabled(p) ((p)->flags & PILOT_DISABLED)


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

	Faction* faction;

	/* object caracteristics */
	Ship* ship; /* ship pilot is flying */
	Solid* solid; /* associated solid (physics) */

	/* current health */
	double armour, shield, energy;
	double armour_max, shield_max, energy_max;
	double fuel; /* used only for jumps, TODO make it matter :) */

	/* associated functions */
	void (*think)(struct Pilot*); /* AI thinking for the pilot */
	void (*update)(struct Pilot*, const double); /* updates the pilot */
	void (*render)(struct Pilot*); /* for rendering the pilot */

	/* outfit management */
	PilotOutfit* outfits;
	int noutfits;
	PilotOutfit* secondary; /* secondary weapon */
	PilotOutfit* ammo; /* secondary ammo if needed */

	/* misc */
	unsigned int flags; /* used for AI and others */
	unsigned int ptimer; /* generic timer for internal pilot use */

	/* AI */
	AI_Profile* ai; /* ai personality profile */
	unsigned int tcontrol; /* timer for control tick */
	unsigned int timer[MAX_AI_TIMERS]; /* timers for AI */
	Task* task; /* current action */
} Pilot;


/*    
 * fleets
 */   
typedef struct {
	Ship* ship; /* ship the pilot is flying */
	char* name; /* used if they have a special name like uniques */
	int chance; /* chance of this pilot appearing in the leet */
} FleetPilot;
typedef struct {
	char* name; /* fleet name, used as the identifier */
	Faction* faction; /* faction of the fleet */

	AI_Profile* ai; /* AI profile to use */

	FleetPilot* pilots; /* the pilots in the fleet */
	int npilots; /* total number of pilots */
} Fleet;


/*
 * getting pilot stuff
 */
extern Pilot* player; /* the player */
Pilot* pilot_get( const unsigned int id );
unsigned int pilot_getNext( const unsigned int id );
unsigned int pilot_getNearest( const Pilot* p );
unsigned int pilot_getHostile (void); /* only for the player */
Fleet* fleet_get( const char* name );


/*
 * misc
 */
void pilot_shoot( Pilot* p, const unsigned int target, const int secondary );
void pilot_hit( Pilot* p, const double damage_shield, const double damage_armour );
void pilot_setAmmo( Pilot* p );
double pilot_face( Pilot* p, const float dir );


/*
 * creation
 */
void pilot_init( Pilot* dest, Ship* ship, char* name, Faction* faction, AI_Profile* ai,
		const double dir, const Vector2d* pos, const Vector2d* vel, const int flags );
unsigned int pilot_create( Ship* ship, char* name, Faction* faction, AI_Profile* ai,
		const double dir, const Vector2d* pos, const Vector2d* vel, const int flags );

/*
 * init/cleanup
 */
void pilot_destroy(Pilot* p);
void pilots_free (void);
void pilots_clean (void);
int fleet_load (void); /* TODO make this fleet stuff actually matter */
void fleet_free (void);


/*
 * update
 */
void pilots_update( double dt );
void pilots_render (void);


#endif /* PILOT_H */
