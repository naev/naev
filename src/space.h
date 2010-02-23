/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SPACE_H
#  define SPACE_H


#include "faction.h"
#include "opengl.h"
#include "pilot.h"
#include "economy.h"
#include "fleet.h"
#include "mission.h"
#include "tech.h"


#define MAX_HYPERSPACE_VEL    25 /**< Speed to brake to before jumping. */


/**
 * @brief Different planet classes.
 *
 * Planets types, taken from
 * http://en.wikipedia.org/wiki/Star_Trek_planet_classifications
 */
typedef enum PlanetClass_ {
   PLANET_CLASS_NULL=0, /**< Null/Not defined */
   PLANET_CLASS_A,   /**< Geothermal */
   PLANET_CLASS_B,   /**< Geomorteus */
   PLANET_CLASS_C,   /**< Geoinactive */
   PLANET_CLASS_D,   /**< Asteroid/Moon */
   PLANET_CLASS_E,   /**< Geoplastic */
   PLANET_CLASS_F,   /**< Geometallic */
   PLANET_CLASS_G,   /**< GeoCrystaline */
   PLANET_CLASS_H,   /**< Desert */
   PLANET_CLASS_I,   /**< Gas Supergiant */
   PLANET_CLASS_J,   /**< Gas Giant */
   PLANET_CLASS_K,   /**< Adaptable */
   PLANET_CLASS_L,   /**< Marginal */
   PLANET_CLASS_M,   /**< Terrestrial */
   PLANET_CLASS_N,   /**< Reducing */
   PLANET_CLASS_O,   /**< Pelagic - Aquatic */
   PLANET_CLASS_P,   /**< Glaciated */
   PLANET_CLASS_Q,   /**< Variable */
   PLANET_CLASS_R,   /**< Rogue */
   PLANET_CLASS_S,   /**< Ultragiant */
   PLANET_CLASS_T,   /**< Ultragiant */
   PLANET_CLASS_X,   /**< Demon */
   PLANET_CLASS_Y,   /**< Demon */
   PLANET_CLASS_Z,   /**< Demon */
   STATION_CLASS_A,  /**< Civilian Station */
   STATION_CLASS_B,  /**< Military Station */
   STATION_CLASS_C,  /**< Interfactional Station */
   STATION_CLASS_D   /**< Robotic Station */
} PlanetClass;

/*
 * planet services
 */
#define PLANET_SERVICE_LAND         (1<<0) /**< Can land. */
#define PLANET_SERVICE_INHABITED    (1<<1) /**< Planet is inhabited. */
#define PLANET_SERVICE_REFUEL       (1<<2) /**< Has refueling. */
#define PLANET_SERVICE_BAR          (1<<3) /**< Has bar and thus news. */
#define PLANET_SERVICE_MISSIONS     (1<<4) /**< Has mission computer. */
#define PLANET_SERVICE_COMMODITY    (1<<5) /**< Can trade commodities. */
#define PLANET_SERVICE_OUTFITS      (1<<6) /**< Can trade outfits. */
#define PLANET_SERVICE_SHIPYARD     (1<<7) /**< Can trade ships. */
#define planet_hasService(p,s)      ((p)->services & s) /**< Checks if planet has service. */


/**
 * @struct Planet
 *
 * @brief Represents a planet.
 */
typedef struct Planet_ {
   int id; /**< Planet ID. */
   char* name; /**< planet name */
   Vector2d pos; /**< position in star system */

   /* Planet details. */
   PlanetClass class; /**< planet type */
   int faction; /**< planet faction */
   int population; /**< Population of the planet. */
   double prodfactor; /**< Default Production factor of the planet. */
   double cur_prodfactor; /**< Current real production factor of the planet. */
  
   /* Landing details. */
   char* description; /**< planet description */
   char* bar_description; /**< spaceport bar description */
   unsigned int services; /**< what services they offer */
   Commodity **commodities; /**< what commodities they sell */
   int ncommodities; /**< the amount they have */
   int bribed; /**< If planet has been bribed. */
   tech_group_t *tech; /**< Planet tech. */

   /* Graphics. */
   glTexture* gfx_space; /**< graphic in space */
   char *gfx_spacePath; /**< Name of the gfx_space for saving purposes. */
   char *gfx_exterior; /**< Don't actually load the texture */
   char *gfx_exteriorPath; /**< Name of the gfx_exterior for saving purposes. */
} Planet;


/* 
 * star system flags
 */
#define SYSTEM_KNOWN       (1<<0) /**< System is known. */
#define SYSTEM_MARKED      (1<<1) /**< System is marked by a regular mission. */
#define SYSTEM_CMARKED     (1<<2) /**< System is marked by a computer mission. */
#define sys_isFlag(s,f)    ((s)->flags & (f)) /**< Checks system flag. */
#define sys_setFlag(s,f)   ((s)->flags |= (f)) /**< Sets a system flag. */
#define sys_rmFlag(s,f)    ((s)->flags &= ~(f)) /**< Removes a system flag. */
#define sys_isKnown(s)     sys_isFlag(s,SYSTEM_KNOWN) /**< Checks if system is known. */
#define sys_isMarked(s)    sys_isFlag(s,SYSTEM_MARKED) /**< Checks if system is marked. */


/*
 * Forward declaration.
 */
struct StarSystem_;
typedef struct StarSystem_ StarSystem;


/**
 * @struct SystemFleet
 *
 * @brief Represents a fleet that can appear in the system.
 */
typedef struct SystemFleet_ {
   Fleet* fleet; /**< fleet to appear */
   int chance; /**< chance of fleet appearing in the system */
} SystemFleet;


/*
 * Jump point flags.
 */
#define JP_AUTOPOS      (1<<0) /**< Automatically position jump point based on system radius. */
#define JP_DISABLED     (1<<1) /**< Jump point is disabled. */
#define JP_HIDDEN       (1<<2) /**< Jump point is hidden by default. */



/**
 * @struct JumpPoint
 *
 * @brief Represents a jump lane.
 */
typedef struct JumpPoint_ {
   StarSystem *target; /**< Target star system to jump to. */
   int targetid; /**< ID of the target star system. */
   Vector2d pos; /**< Position in the system. */
   double radius; /**< Radius of jump range. */
   unsigned int flags; /**< Flags related to the jump point's status. */
   int known; /**< Is the jump point known? */
   double angle; /**< Direction the jump is facing. */
   double cosa; /**< Cosinus of the angle. */
   double sina; /**< Sinus of the angle. */
   int sx; /**< X sprite to use. */
   int sy; /**< Y sprite to use. */
} JumpPoint;
extern glTexture *jumppoint_gfx; /**< Jump point graphics. */

/**
 * @struct StarSystem
 *
 * @brief Represents a star system.
 *
 * The star system is the basic setting in NAEV.
 */
struct StarSystem_ {
   int id; /**< Star system index. */

   /* General. */
   char* name; /**< star system name */
   Vector2d pos; /**< position */
   int stars; /**< Amount of "stars" it has. */
   int asteroids; /**< @todo implement asteroids */
   double interference; /**< in % @todo implement interference. */
   double nebu_density; /**< Nebula density (0. - 1000.) */
   double nebu_volatility; /**< Nebula volatility (0. - 1000.) */
   double radius; /**< Default system radius for standard jump points. */

   /* Planets. */
   Planet **planets; /**< planets */
   int *planetsid; /**< IDs of the planets. */
   int nplanets; /**< total number of planets */
   int faction; /**< overall faction */

   /* Jumps. */
   JumpPoint *jumps; /**< Jump points in the system */
   int njumps; /**< number of adjacent jumps */

   /* Fleets. */
   SystemFleet* fleets; /**< fleets that can appear in the current system */
   int nfleets; /**< total number of fleets */
   double avg_pilot; /**< Target amount of pilots in the system. */

   /* Fleet data - @TODO Remove in favour of more sane fleet spawn system. */
   char **fltdat;
   int nfltdat;

   /* Calculated. */
   double *prices; /**< Handles the prices in the system. */
   double security; /**< % of security in this system. */

   /* Markers. */
   int markers_misc; /**< Number of misc mission markers on system. */
   int markers_rush; /**< Number of rush mission markers on system. */
   int markers_cargo; /**< Number of cargo mission markers on system. */

   /* Misc. */
   unsigned int flags; /**< flags for system properties */
};


extern StarSystem *cur_system; /**< current star system */
extern int space_spawn; /**< 1 if spawning is enabled. */


/*
 * loading/exiting
 */
void space_initStars( int n );
void space_init( const char* sysname );
int space_load (void);
void space_exit (void);

/*
 * planet stuff
 */
Planet *planet_new (void);
char* planet_getSystem( const char* planetname );
Planet* planet_getAll( int *n );
Planet* planet_get( const char* planetname );
Planet* planet_getIndex( int ind );
int planet_getNum (void);
int planet_exists( const char* planetname );
char planet_getClass( const Planet *p );

/*
 * system adding/removing stuff.
 */
void systems_reconstructJumps (void);
void systems_reconstructPlanets (void);
StarSystem *system_new (void);
int system_addPlanet( StarSystem *sys, const char *planetname );
int system_rmPlanet( StarSystem *sys, const char *planetname );
int system_addFleet( StarSystem *sys, SystemFleet *fleet );
int system_rmFleet( StarSystem *sys, SystemFleet *fleet );
int system_addFleetGroup( StarSystem *sys, FleetGroup *fltgrp );
int system_rmFleetGroup( StarSystem *sys, FleetGroup *fltgrp );

/*
 * render
 */
void space_renderStars( const double dt );
void space_render( const double dt );
void space_renderOverlay( const double dt );
void planets_render (void);

/*
 * update
 */
void space_update( const double dt );

/* 
 * Getting stuff.
 */
const StarSystem* system_getAll( int *nsys );
StarSystem* system_get( const char* sysname );
StarSystem* system_getIndex( int id );
int space_sysReachable( StarSystem *sys );
char** space_getFactionPlanet( int *nplanets, int *factions, int nfactions );
char* space_getRndPlanet (void);


/* 
 * Markers.
 */
int space_addMarker( const char *sys, SysMarker type );
int space_rmMarker( const char *sys, SysMarker type );
void space_clearKnown (void);
void space_clearMarkers (void);
void space_clearComputerMarkers (void);


/* 
 * Hyperspace.
 */
int space_canHyperspace( Pilot* p);
int space_hyperspace( Pilot* p );
int space_calcJumpInPos( StarSystem *in, StarSystem *out, Vector2d *pos, Vector2d *vel, double *dir );


/* Stardate. */
extern char* stardate;


#endif /* SPACE_H */
