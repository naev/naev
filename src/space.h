/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SPACE_H
#  define SPACE_H


#include "faction.h"
#include "opengl.h"
#include "pilot.h"
#include "economy.h"


#define MIN_HYPERSPACE_DIST   1500
#define MAX_HYPERSPACE_VEL    25


#define PLANET_TECH_MAX       8


/*
 * Planets types, taken from
 * http://en.wikipedia.org/wiki/Star_Trek_planet_classifications
 */
typedef enum PlanetClass_ {
   PLANET_CLASS_NULL=0, /* Null/Not defined */
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
   PLANET_CLASS_Z,   /* Demon */
   STATION_CLASS_A,  /* Civilian Station */
   STATION_CLASS_B,  /* Military Station */
   STATION_CLASS_C,  /* Interfactional Station */
   STATION_CLASS_D  /* Robotic Station */
} PlanetClass;

/*
 * planet services
 */
#define PLANET_SERVICE_LAND         (1<<0) /* can land */
#define PLANET_SERVICE_BASIC        (1<<1) /* refueling, spaceport bar, news */
#define PLANET_SERVICE_COMMODITY    (1<<2)
#define PLANET_SERVICE_OUTFITS      (1<<3)
#define PLANET_SERVICE_SHIPYARD     (1<<4)
#define planet_hasService(p,s)      ((p)->services & s)

typedef struct Planet_ {
   char* name; /* planet name */
   Vector2d pos; /* position in star system */

   PlanetClass class; /* planet type */
   int faction; /* planet faction */
   
   char* description; /* planet description */
   char* bar_description; /* spaceport bar description */
   unsigned int services; /* what services they offer */
   Commodity **commodities; /* what commodities they sell */
   int ncommodities; /* the amount they have */

   /* tech[0] stores global tech level (everything that and below) while
    * tech[1-PLANET_TECH_MAX] store the "unique" tech levels (only matches */
   int tech[PLANET_TECH_MAX];

   glTexture* gfx_space; /* graphic in space */
   glTexture* gfx_exterior; /* graphic in the exterior */
} Planet;


/* 
 * star system flags
 */
#define SYSTEM_KNOWN    (1<<0)
#define SYSTEM_MARKED   (1<<1)
#define sys_isFlag(s,f)    ((s)->flags & (f))
#define sys_setFlag(s,f)   if (!sys_isFlag(s,f)) (s)->flags |= (f)
#define sys_rmFlag(s,f)    if (sys_isFlag(s,f)) (s)->flags ^= (f)
#define sys_isKnown(s)     sys_isFlag(s,SYSTEM_KNOWN)
#define sys_isMarked(s)    sys_isFlag(s,SYSTEM_MARKED)

/*
 * star systems                                                   
 */
typedef struct SystemFleet_ {
   Fleet* fleet; /* fleet to appear */
   int chance; /* chance of fleet appearing in the system */
} SystemFleet;
typedef struct StarSystem_ {
   char* name; /* star system identifier */

   Vector2d pos; /* position */
   int stars, asteroids; /* in number */
   double interference; /* in % */

   int faction; /* overall faction */

   Planet *planets; /* planets */
   int nplanets; /* total number of planets */

   SystemFleet* fleets; /* fleets that can appear in the current system */
   int nfleets; /* total number of fleets */

   int *jumps; /* adjacent star system index numbers */
   int njumps; /* number of adjacent jumps */

   double nebu_density; /* Nebulae density */
   double nebu_volatility; /* Nebulae volatility - Not used yet */

   unsigned int flags; /* flags for system properties */
} StarSystem;


extern StarSystem *cur_system; /* current star system */


/*
 * loading/exiting
 */
void space_init( const char* sysname );
int space_load (void);
void space_exit (void);

/*
 * planet stuff
 */
char* planet_getSystem( char* planetname );
Planet* planet_get( char* planetname );

/*
 * render
 */
void space_render( const double dt );
void space_renderOverlay( const double dt );
void planets_render (void);

/*
 * update
 */
void space_update( const double dt );

/*
 * misc
 */
StarSystem* system_get( const char* sysname );
int space_canHyperspace( Pilot* p);
int space_hyperspace( Pilot* p );
int space_sysReachable( StarSystem *sys );
char** space_getFactionPlanet( int *nplanets, int *factions, int nfactions );
char* space_getRndPlanet (void);
void space_clearKnown (void);
void space_clearMarkers (void);
extern char* stardate;


#endif /* SPACE_H */
