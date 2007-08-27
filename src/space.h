

#ifndef SPACE_H
#  define SPACE_H


#include "faction.h"
#include "opengl.h"
#include "pilot.h"


#define MIN_HYPERSPACE_DIST	1500
#define MAX_HYPERSPACE_VEL		15


/*
 * Planets types, taken from
 * http://en.wikipedia.org/wiki/Star_Trek_planet_classifications
 */
typedef enum { PLANET_CLASS_NULL=0, /* Null/Not defined */
	PLANET_CLASS_A,   /* Geothermal */
	PLANET_CLASS_B,   /* Geomorteus */
	PLANET_CLASS_C,   /* Geoinactive */
	PLANET_CLASS_D,   /* Asteroid/Moon */
	PLANET_CLASS_E,   /* Geoplastic */
	PLANET_CLASS_F,   /* Geometallic */
	PLANET_CLASS_G,   /* GeoCrystaline */
	PLANET_CLASS_H,   /* Desert */
	PLANET_CLASS_I,   /* Gas Supergiant */
	PLANET_CLASS_J,   /* Gas Giant */
	PLANET_CLASS_K,   /* Adaptable */
	PLANET_CLASS_L,   /* Marginal */
	PLANET_CLASS_M,   /* Terrestrial */
	PLANET_CLASS_N,   /* Reducing */
	PLANET_CLASS_O,   /* Pelagic */
	PLANET_CLASS_P,   /* Glaciated */
	PLANET_CLASS_Q,   /* Variable */
	PLANET_CLASS_R,   /* Rogue */
	PLANET_CLASS_S,   /* Ultragiant */
	PLANET_CLASS_T,   /* Ultragiant */
	PLANET_CLASS_X,   /* Demon */
	PLANET_CLASS_Y,   /* Demon */
	PLANET_CLASS_Z    /* Demon */
} PlanetClass;

/*
 * planet services
 */
#define PLANET_SERVICE_BASIC			(1<<0) /* refueling, spaceport bar, news */
#define PLANET_SERVICE_COMMODITY		(1<<1)
#define PLANET_SERVICE_OUTFITS		(1<<2)
#define PLANET_SERVICE_SHIPYARD		(1<<3)
#define planet_hasService(p,s)		((p)->services & s)

typedef struct {
	char* name; /* planet name */
	Vector2d pos; /* position in star system */

	PlanetClass class; /* planet type */
	Faction* faction; /* planet faction */
	
	char* description; /* planet description */
	char* bar_description; /* spaceport bar description */
	int services; /* what services they offer */

	glTexture* gfx_space; /* graphic in space */
	glTexture* gfx_exterior; /* graphic in the exterior */
} Planet;


/*
 * star systems                                                           
 */
typedef struct {
	Fleet* fleet; /* fleet to appear */
	int chance; /* chance of fleet appearing in the system */
} SystemFleet;
typedef struct {
	char* name; /* star system identifier */

	Vector2d pos; /* position */
	int stars, asteroids; /* in number */
	double interference; /* in % */

	Planet *planets; /* planets */
	int nplanets; /* total number of planets */

	SystemFleet* fleets; /* fleets that can appear in the current system */
	int nfleets; /* total number of fleets */
} StarSystem;


extern StarSystem *cur_system; /* current star system */


/*
 * loading/exiting
 */
void space_init( const char* sysname );
int space_load (void);
void space_exit (void);

/*
 * render
 */
void space_render( double dt );
void planets_render (void);

/*
 * misc
 */
int space_hyperspace( Pilot* p );
extern char* stardate;


#endif /* SPACE_H */
